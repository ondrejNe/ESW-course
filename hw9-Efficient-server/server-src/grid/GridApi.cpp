#include "GridModel.hh"

const std::vector<std::pair<int64_t, int64_t>> precomputedNeighbourPairs{
        {-1, -1}, {-1,  0}, {-1,  1},
        { 0, -1}, { 0,  1}, { 1, -1},
        { 1,  0}, { 1,  1}
};

string Grid::getPointCellId(Point &point) {
    pair <uint64_t, uint64_t> probableCellCoords = make_pair(point.x / 500, point.y / 500);

    // Search whether there isn't a better match
    locker.sharedLock();

    for (const auto &comb : precomputedNeighbourPairs) {
        std::string neighborCellId = valuesToCellId(probableCellCoords.first + comb.first,
                                                    probableCellCoords.second + comb.second);
        auto cellIt = cells.find(neighborCellId);
        if (cellIt == cells.end()) continue; // The searched cell does not exist

        Point neighborPoint = *cellIt->second.points.begin();
        if (euclideanDistance(point, neighborPoint) <= 250000) {
            locker.sharedUnlock();
            return neighborCellId;
        }
    }

    locker.sharedUnlock();

    return valuesToCellId(probableCellCoords.first, probableCellCoords.second);
}

void Grid::addPoint(Point &point, string &cellId) {
    // Add the point to the cell
    apiLogger.debug("Add point <%d,%d> to cell [%s]", point.x, point.y, cellId.c_str());
    locker.uniqueLock();

    if (cells.count(cellId) == 0) {
        // Create a new cell
        Cell newCell = pointToCell(point);
        newCell.points.insert(point);
        cells[cellId] = newCell;

    } else {
        // Add point to existing cell
        cells[cellId].points.insert(point);
    }

    locker.uniqueUnlock();
}

void Grid::addEdge(string &originCellId, string &destinationCellId, uint64_t length) {
    apiLogger.debug("Add edge [%s] -> [%s] with length %d", originCellId.c_str(), destinationCellId.c_str(), length);

    // Insert length
    locker.uniqueLock();
    // Does the edge already exist?
    if (cells[originCellId].edges.count(destinationCellId) == 0) {
        // Add new edge
        cells[originCellId].edges[destinationCellId] = length;
    } else {
        // Update existing edge
        uint64_t stored = cells[originCellId].edges[destinationCellId];
        cells[originCellId].edges[destinationCellId] = (stored + length) / 2;
    }

    locker.uniqueUnlock();
}

void Grid::resetGrid() {
    locker.uniqueLock();

    for (auto &cell: cells) {
        cell.second.points.clear();
        cell.second.edges.clear();
    }
    cells.clear();

    locker.uniqueUnlock();
}
