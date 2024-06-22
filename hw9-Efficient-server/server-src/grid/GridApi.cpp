
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define GRID_LOGGER
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

uint64_t Grid::getPointCellId(Point &point) {
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

void Grid::addPoint(Point &point, uint64_t &cellId) {
    location_count++;

    auto it = cells.find(cellId);

    if (it == cells.end()) {
        uint64_t coordX = point.x / 500;
        uint64_t coordY = point.y / 500;
        uint64_t id = ((coordX << 32) | coordY);

        tsl::robin_map<uint64_t, Stat> newStats = tsl::robin_map<uint64_t, Stat>();
        newStats.reserve(115000);
        Cell newCell = {id, coordX, coordY, point.x, point.y, newStats};
        cells[id] = newCell;
    }
}

void Grid::addEdge(uint64_t &originCellId, uint64_t &destinationCellId, uint64_t length) {
    Cell &originCell = cells[originCellId];
    uint64_t &destinationEdge = originCell.stats[destinationCellId].edge;

    if (destinationEdge == 0) {
        destinationEdge = length;
        edges_count += 1;
    } else {
        destinationEdge = (destinationEdge + length) / 2;
    }
}

void Grid::resetGrid() {
    cells.clear();

    edges_count = 0;
    edges_space = 0;
    location_count = 0;
    walk_count = 0;
    oneToOne_count = 0;
    oneToAll_count = 0;
}

void Grid::logGridGraph() {
#ifdef GRID_LOGGER
    // Log information about cells
    gridLogger.info("Grid contains %lu cells:", cells.size());
    for (const auto& [cellId, cell] : cells) {
        gridLogger.info("Cell %lu: Coord(%lu, %lu), Point(%lu, %lu)",
                        cell.id, cell.coordX, cell.coordY, cell.pointX, cell.pointY);
        for (const auto& [id, stats] : cell.stats) {
            gridLogger.info("  with Stat to Cell %lu edge: %lu, distance: %lu", id, stats.edge, stats.distance);
        }
    }
#endif
}
