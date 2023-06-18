#include "DataGrid.hh"

// #define LOCKING

const vector<pair<uint64_t, uint64_t>> precomputedNeighbourPairs{
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1}, { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
};

void DataGrid::addPoint(Point &point, string &cellId) {
    // Add the point to the cell
    {
        #ifdef LOCKING
        unique_lock<shared_mutex> lock(rwMutex);
        #endif

        if (cells.count(cellId) == 0) {
            // Create a new cell
            Cell newCell = pointToCell(point);
            newCell.points.insert(point);
            cells[cellId] = newCell;

        } 
        // else {
        //     // Add point to existing cell
        //     cells[cellId].points.insert(point);
        // }
        
        #ifdef LOCKING
        lock.unlock();
        #endif
    }
}

string DataGrid::getPointCellId(Point &point) {
    pair <uint64_t, uint64_t> probableCellCoords = pointToCellCoords(point);

    // Search whether there isn't a better match
    {
        #ifdef LOCKING
        shared_lock<shared_mutex> lock(rwMutex);
        #endif

        for (auto &comb: precomputedNeighbourPairs) {
            string neighborCellId = valuesToCellId(probableCellCoords.first + comb.first, probableCellCoords.second + comb.second);

            if (cells.count(neighborCellId) == 0) continue; // The searched cell does not exist

            Point neighborPoint = *cells[neighborCellId].points.begin();
            if (euclideanDistance(point, neighborPoint) <= 250000) {

                #ifdef LOCKING
                lock.unlock(); // First unlock
                #endif
                return neighborCellId;
            }
        }

        #ifdef LOCKING
        lock.unlock(); // Second unlock
        #endif
    }

    return valuesToCellId(probableCellCoords.first, probableCellCoords.second);
}

void DataGrid::addEdge(Point &origin, Point &destination, uint64_t length) {
    // Find the cells where the points belong
    string originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    string destinationCellId = getPointCellId(destination);
    addPoint(destination, destinationCellId);

    // Insert length
    {
        #ifdef LOCKING
        unique_lock<shared_mutex> lock(rwMutex);
        #endif
        // Does the edge already exist?
        if (cells[originCellId].edges.count(destinationCellId) == 0) {
            // Add new edge
            cells[originCellId].edges[destinationCellId] = length;
        } else {
            // Update existing edge
            uint64_t stored = cells[originCellId].edges[destinationCellId];
            cells[originCellId].edges[destinationCellId] = (stored + length) / 2;
        }

        #ifdef LOCKING
        lock.unlock();
        #endif
    }
}

void DataGrid::resetDataGrid() 
{
    #ifdef LOCKING
    unique_lock<shared_mutex> lock(rwMutex);
    #endif

    for (auto& cell : cells) {
        cell.second.points.clear();
        cell.second.edges.clear();
    }
    cells.clear();

    #ifdef LOCKING
    lock.unlock();
    #endif
}
