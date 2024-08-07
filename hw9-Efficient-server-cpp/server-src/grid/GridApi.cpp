
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define GRID_GRAPH_LOGGER
//#define GRID_STATS_LOGGER
//#define GRID_EDGE_LOGGER
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
        auto cellIt = cells[neighborCellId % CHUNKS].find(neighborCellId);
        if (cellIt == cells[neighborCellId % CHUNKS].end()) continue; // The searched cell does not exist

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

    auto it = cells[cellId % CHUNKS].find(cellId);

    if (it == cells[cellId % CHUNKS].end()) {
        uint64_t coordX = point.x / 500;
        uint64_t coordY = point.y / 500;
        uint64_t id = ((coordX << 32) | coordY);

        vector<Edge> newEdges = vector<Edge>();
        newEdges.reserve(5);
        vector<Edge> newInEdges = vector<Edge>();
        newInEdges.reserve(5);
        Cell newCell = {id, coordX, coordY, point.x, point.y, newEdges, newInEdges};
        cells[cellId % CHUNKS][id] = newCell;

        gridStats.quad[cellId % CHUNKS]++;

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
    for (auto &[id, len, samples]: cells[originCellId % CHUNKS][originCellId].edges) {
        if (id == destinationCellId) {
            len += length;
            samples++;
            return;
        }
    }
    for (auto &[id, len, samples]: cells[destinationCellId % CHUNKS][destinationCellId].inEdges) {
        if (id == originCellId) {
            len += length;
            samples++;
            return;
        }
    }

    gridStats.edges_count++;
    cells[originCellId % CHUNKS][originCellId].edges.push_back({destinationCellId, length, 1});
    cells[destinationCellId % CHUNKS][destinationCellId].inEdges.push_back({originCellId, length, 1});
}

void GridData::resetGrid(GridStats &gridStats) {
    for (int i = 0; i < CHUNKS; i++) {
        cells[i].clear();
        gridStats.quad[i] = 0;
    }

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
#ifdef GRID_EDGE_LOGGER
    std::map<pair<uint64_t, uint64_t>, uint64_t> edgeStats;
    for (const auto& batch : cells) {
        for (const auto& [cellId, cell] : batch) {
            uint64_t in = cell.edges.size();
            uint64_t out = cell.inEdges.size();
            edgeStats[{in, out}]++;
        }
    }
    for (const auto& [type, count] : edgeStats) {
        gridLogger.info("  Cells with in: %lu out: %lu num: %lu", type.first, type.second, count);
    }
#endif
}

void GridStats::logGridStats() {
#ifdef GRID_STATS_LOGGER
    gridLogger.info("  Edges count: %lu", edges_count);
    gridLogger.info("  Highest X: %lu, %lu", highestCoordX.first, highestCoordX.second);
    gridLogger.info("  Highest Y: %lu, %lu", highestCoordY.first, highestCoordY.second);
    gridLogger.info("  Lowest X: %lu, %lu", lowestCoordX.first, lowestCoordX.second);
    gridLogger.info("  Lowest Y: %lu, %lu", lowestCoordY.first, lowestCoordY.second);
#endif
    gridLogger.info("  Walks: %lu OneToOne: %lu OneToAll: %lu Locations: %lu", walk_count, oneToOne_count,
                    oneToAll_count, location_count);
}
