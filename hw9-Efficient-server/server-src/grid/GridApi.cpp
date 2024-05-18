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

string Grid::getPointCellId(Point &point) {
    pair <uint64_t, uint64_t> probableCellCoords = make_pair(point.x / 500, point.y / 500);
    string probableCellId = valuesToCellId(probableCellCoords.first, probableCellCoords.second);
    // Search whether there isn't a better match
    locker.uniqueLock();

    for (auto &comb: precomputedNeighbourPairs) {
        string neighborCellId = valuesToCellId(probableCellCoords.first + comb.first,
                                               probableCellCoords.second + comb.second);

        if (cells.count(neighborCellId) == 0) continue; // The searched cell does not exist

        Point neighborPoint = *cells[neighborCellId].points.begin();
        if (euclideanDistance(point, neighborPoint) <= 250000) {
            // Add the point to the cell
            cells[neighborCellId].points.insert(point);

            locker.uniqueUnlock();
            return neighborCellId;
        }
    }

    // Add a new cell and point to it
    Cell newCell = pointToCell(point);
    newCell.points.insert(point);
    cells[probableCellId] = newCell;

    locker.uniqueUnlock();

    return probableCellId;
}

void Grid::addEdge(Point &origin, Point &destination, uint64_t length) {
    // Find the cells where the points belong
    string originCellId = getPointCellId(origin);
    string destinationCellId = getPointCellId(destination);

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
