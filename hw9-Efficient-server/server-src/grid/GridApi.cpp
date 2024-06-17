#include "GridModel.hh"

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

        uint64_t &neighborPointX = cellIt->second.pointX;
        uint64_t &neighborPointY = cellIt->second.pointY;
        uint64_t dx = 0;
        if (point.x > neighborPointX) {
            dx = point.x - neighborPointX;
        } else {
            dx = neighborPointX - point.x;
        }
        uint64_t dy = 0;
        if (point.y > neighborPointY) {
            dy = point.y - neighborPointY;
        } else {
            dy = neighborPointY - point.y;
        }
        if ((dx * dx + dy * dy) <= 250000) {
            return neighborCellId;
        }
    }

    return (probableCoordX << 32) | (probableCoordY);
}

void Grid::addPoint(Point &point, uint64_t &cellId) {
    auto it = cells.find(cellId);

    if (it == cells.end()) {
        uint64_t coordX = point.x / 500;
        uint64_t coordY = point.y / 500;
        uint64_t id = (coordX << 32) | coordY;
        Cell newCell = {id, coordX, coordY, point.x, point.y};
        cells[id] = newCell;
        edges[id] = robin_hood::unordered_map<uint64_t, uint64_t>();
        edges[id].reserve(10);
        distances[id] = robin_hood::unordered_map<uint64_t, uint64_t>();
        distances[id].reserve(10);
    }
}

void Grid::addEdge(uint64_t &originCellId, uint64_t &destinationCellId, uint64_t length) {
    uint64_t &edgeLength = edges[originCellId][destinationCellId];
    if (edgeLength == 0) {
        edgeLength = length;
        edges_count += 1;
    } else {
        edgeLength = (edgeLength + length) / 2;
    }
}

void Grid::resetGrid() {
    cells.clear();
    distances.clear();
    edges.clear();
}
