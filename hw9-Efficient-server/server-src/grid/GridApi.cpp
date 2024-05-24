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
    apiLogger.debug("Getting cell id for point (%lu, %lu)", point.x, point.y);
    pair<uint64_t, uint64_t> probableCellCoords = {point.x / 500, point.y / 500};
    uint64_t probableCellId = ((point.x / 500) << 32) | ((point.y / 500));
    apiLogger.debug("Probable cell id: %lu", probableCellId);

    // Search whether there isn't a better match
    for (const auto &comb: precomputedNeighbourPairs) {
        uint64_t neighborCellId =
                ((probableCellCoords.first + comb.first) << 32) | (probableCellCoords.second + comb.second);
        auto cellIt = cells.find(neighborCellId);
        if (cellIt == cells.end()) continue; // The searched cell does not exist

        Point &neighborPoint = cellIt->second.point;
        if (euclideanDistance(point, neighborPoint) <= 250000) {
            apiLogger.debug("Neighbor cell id found: %lu with point %lu ; %lu", neighborCellId, neighborPoint.x,
                            neighborPoint.y);
            return neighborCellId;
        }
    }

    apiLogger.debug("Cell id not found, using probable cell id");
    return probableCellId;
}

uint64_t Grid::euclideanDistance(Point &p1, Point &p2) {
    uint64_t dx = std::abs(static_cast<int64_t>(p1.x) - static_cast<int64_t>(p2.x));
    uint64_t dy = std::abs(static_cast<int64_t>(p1.y) - static_cast<int64_t>(p2.y));
    return dx * dx + dy * dy;
}

void Grid::addPoint(Point &point, uint64_t &cellId) {
    auto it = cells.find(cellId);

    if (it == cells.end()) {
        apiLogger.debug("Creating new cell for point (%lu, %lu)", point.x, point.y);
        // Create a new cell
        uint64_t coordX = point.x / 500;
        uint64_t coordY = point.y / 500;
        uint64_t id = (coordX << 32) | coordY;
        apiLogger.debug("New cell id: %lu", id);
        Point p = {point.x, point.y};
        Cell newCell = {id, coordX, coordY, p};
        cells[id] = newCell;
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
