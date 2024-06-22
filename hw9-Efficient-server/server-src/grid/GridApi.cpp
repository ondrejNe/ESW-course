
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define GRID_GRAPH_LOGGER
#define GRID_STATS_LOGGER
PrefixedLogger gridLogger = PrefixedLogger("[GRID      ]", true);

// Class definition -------------------------------------------------------------------------------
const std::vector <std::pair<int64_t, int64_t>> precomputedNeighbourPairs{
        {-1, -1},
        {-1, 0},
        {-1, 1},
        {0,  -1},
        {0,  1},
        {1,  -1},
        {1,  0},
        {1,  1}
};

uint64_t GridData::getPointCellId(Point &point) {
    uint64_t probableCoordX = point.x / 500;
    uint64_t probableCoordY = point.y / 500;

    // Search whether there isn't a better match
    for (const auto &comb: precomputedNeighbourPairs) {
        uint64_t neighborCellId = ((probableCoordX + comb.first) << 32) | (probableCoordY + comb.second);
        auto cellIt = cells.find(neighborCellId);
        if (cellIt == cells.end()) continue; // The searched cell does not exist

        const uint64_t &neighborPointX = cellIt->second.pointX;
        const uint64_t &neighborPointY = cellIt->second.pointY;
        uint64_t dx = (point.x > neighborPointX) ? (point.x - neighborPointX) : (neighborPointX - point.x);
        uint64_t dy = (point.y > neighborPointY) ? (point.y - neighborPointY) : (neighborPointY - point.y);
        if ((dx * dx + dy * dy) <= 250000) {
            return neighborCellId;
        }
    }

    return ((probableCoordX << 32) | (probableCoordY));
}

void GridData::addPoint(GridStats &gridStats, Point &point, uint64_t &cellId) {
    gridStats.location_count++;

    auto it = cells.find(cellId);

    if (it == cells.end()) {
        uint64_t coordX = point.x / 500;
        uint64_t coordY = point.y / 500;
        uint64_t id = ((coordX << 32) | coordY);

        std::unordered_map<uint64_t, Edge> newEdges = std::unordered_map<uint64_t, Edge>();
        newEdges.reserve(5);
        Cell newCell = {id, coordX, coordY, point.x, point.y, newEdges};
        cells[id] = newCell;

#ifdef GRID_STATS_LOGGER
        if (coordX > gridStats.highestCoordX.first) {
            gridStats.highestCoordX = {coordX, coordY};
        }
        if (coordX < gridStats.lowestCoordX.first) {
            gridStats.lowestCoordX = {coordX, coordY};
        }
        if (coordY > gridStats.highestCoordY.second) {
            gridStats.highestCoordY = {coordX, coordY};
        }
        if (coordY < gridStats.lowestCoordY.second) {
            gridStats.lowestCoordY = {coordX, coordY};
        }
#endif
    }
}

void GridData::addEdge(GridStats &gridStats, uint64_t &originCellId, uint64_t &destinationCellId, uint64_t length) {
    Edge &destinationEdge = cells[originCellId].edges[destinationCellId];

    if (destinationEdge.length == 0) {
        gridStats.edges_count++;
    }

    destinationEdge.length += length;
    destinationEdge.samples++;
}

void GridData::resetGrid(GridStats &gridStats) {
    cells.clear();

    gridStats.edges_count = 0;
    gridStats.highestCoordX = {numeric_limits<uint64_t>::min(), 0};
    gridStats.highestCoordY = {0, numeric_limits<uint64_t>::min()};
    gridStats.lowestCoordX = {numeric_limits<uint64_t>::max(), 0};
    gridStats.lowestCoordY = {0, numeric_limits<uint64_t>::max()};

    gridStats.walk_count = 0;
    gridStats.oneToOne_count = 0;
    gridStats.oneToAll_count = 0;
    gridStats.location_count = 0;
}

void GridData::logGridGraph() {
#ifdef GRID_GRAPH_LOGGER
    // Log information about cells
    gridLogger.info("Grid contains %lu cells:", cells.size());
    for (const auto& [cellId, cell] : cells) {
        gridLogger.info("Cell %lu: Coord(%lu, %lu), Point(%lu, %lu)",
                        cell.id, cell.coordX, cell.coordY, cell.pointX, cell.pointY);
        for (const auto& [id, edge] : cell.edges) {
            gridLogger.info("  with Edge to Cell %lu edge: %lu", id, edge.length / edge.samples);
        }
    }
#endif
}

void GridStats::logGridStats() {
#ifdef GRID_STATS_LOGGER
    gridLogger.info("Grid statistics:");
    gridLogger.info("  Edges count: %lu", edges_count);
    gridLogger.info("  Highest X: %lu, %lu", highestCoordX.first, highestCoordX.second);
    gridLogger.info("  Highest Y: %lu, %lu", highestCoordY.first, highestCoordY.second);
    gridLogger.info("  Lowest X: %lu, %lu", lowestCoordX.first, lowestCoordX.second);
    gridLogger.info("  Lowest Y: %lu, %lu", lowestCoordY.first, lowestCoordY.second);
    gridLogger.info("  Walks: %lu", walk_count);
    gridLogger.info("  OneToOne: %lu", oneToOne_count);
    gridLogger.info("  OneToAll: %lu", oneToAll_count);
    gridLogger.info("  Locations: %lu", location_count);
#endif
}
