
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
PrefixedLogger gridLogger = PrefixedLogger("[GRID      ]", false);

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
        Cell newCell = {id, coordX, coordY, point.x, point.y};

        cells[id] = newCell;
        edges[id] = tsl::robin_map<uint64_t, uint64_t>();
        distances[id] = tsl::robin_map<uint64_t, uint64_t>();
        distances[id][id] = 0;
    }
}

void Grid::addEdge(uint64_t &originCellId, uint64_t &destinationCellId, uint64_t length) {
    // Edges
    uint64_t edgeLength = edges[originCellId][destinationCellId];
    if (edgeLength == 0) {
        edgeLength = length;
        edges_count += 1;
    } else {
        edgeLength = (edgeLength + length) / 2;
    }
    edges[originCellId][destinationCellId] = edgeLength;

    // Distances
    uint64_t distanceLength = distances[originCellId][destinationCellId];
    if (distanceLength == 0 || distanceLength > edgeLength) {
        distanceLength = edgeLength;
    }
    distances[originCellId][destinationCellId] = distanceLength;
}

void Grid::resetGrid() {
    cells.clear();
    edges.clear();
    distances.clear();
}

void Grid::logGridGraph() {
    // Log information about cells
    gridLogger.info("Grid contains %lu cells:", cells.size());
    for (const auto& [cellId, cell] : cells) {
        gridLogger.info("Cell %lu: Coord(%lu, %lu), Point(%lu, %lu)",
                        cell.id, cell.coordX, cell.coordY, cell.pointX, cell.pointY);
    }

    // Log information about edges and distances
    gridLogger.info("Grid contains %lu edges:", edges_count);
    for (const auto& [originCellId, edgeMap] : edges) {
        for (const auto& [destinationCellId, length] : edgeMap) {
            gridLogger.info("Edge from Cell %lu to Cell %lu with length %lu",
                            originCellId, destinationCellId, length);
        }
    }

    gridLogger.info("Grid distances:");
    for (const auto& [originCellId, distMap] : distances) {
        for (const auto& [destinationCellId, distance] : distMap) {
            gridLogger.info("Distance from Cell %lu to Cell %lu is %lu",
                            originCellId, destinationCellId, distance);
        }
    }
}
