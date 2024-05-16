#include "GridModel.hh"

const vector <pair<uint64_t, uint64_t>> precomputedNeighbourPairs{
        {-1, -1},
        {-1,  0},
        {-1,  1},
        { 0, -1},
        { 0,  1},
        { 1, -1},
        { 1,  0},
        { 1,  1}
};

void Grid::addPoint(Point &point, string &cellId) {
    // Add the point to the cell
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

string Grid::getPointCellId(Point &point) {
    pair <uint64_t, uint64_t> probableCellCoords = pointToCellCoords(point);

    // Search whether there isn't a better match
    locker.sharedLock();

    for (auto &comb: precomputedNeighbourPairs) {
        string neighborCellId = valuesToCellId(probableCellCoords.first + comb.first,
                                               probableCellCoords.second + comb.second);

        if (cells.count(neighborCellId) == 0) continue; // The searched cell does not exist

        Point neighborPoint = *cells[neighborCellId].points.begin();
        if (euclideanDistance(point, neighborPoint) <= 250000) {

            locker.sharedUnlock();
            return neighborCellId;
        }
    }

    locker.sharedUnlock();

    return valuesToCellId(probableCellCoords.first, probableCellCoords.second);
}

void Grid::addEdge(Point &origin, Point &destination, uint64_t length) {
    // Find the cells where the points belong
    string originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    string destinationCellId = getPointCellId(destination);
    addPoint(destination, destinationCellId);

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
