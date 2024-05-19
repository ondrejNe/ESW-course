
#include "GridModel.hh"

uint64_t Grid::allDijkstra(string &originCellId) {
    uint64_t sum = 0;

    locker.sharedLock();
    searchLogger.info("All Dijkstra cell count: %d", cells.size());

    for (const auto &entry: cells) {
        string destinationCellId = entry.first;
        uint64_t shortestPath = this->dijkstra(originCellId, destinationCellId);
        if (shortestPath == numeric_limits<uint64_t>::max()) {
            sum += 0;
        } else {
            sum += shortestPath;
        }
    }

    locker.sharedUnlock();

    return sum;
}

uint64_t Grid::dijkstra(string &originCellId, string &destinationCellId) {
    searchLogger.debug("Shortest path from [%s] to [%s]", originCellId.c_str(), destinationCellId.c_str());

    // Priority queue to store cells to be processed based on their distances
    priority_queue < pair < uint64_t, string >, vector < pair < uint64_t, string >>, greater<>> pq;
    // Map to store distances from the source to each cell
    unordered_map <string, uint64_t> distances;

    locker.sharedLock();

    // Set all distances to infinity except for the source cell (set to 0)
    for (const auto &entry: cells) {
        distances[entry.first] = numeric_limits<uint64_t>::max();
    }
    distances[originCellId] = 0;

    // Add the source cell to the priority queue
    pq.push(make_pair(distances[originCellId], originCellId));

    searchLogger.debug("pq added entry <%d,[%s]>", distances[originCellId], originCellId.c_str());

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        string currentCellId = pq.top().second;
        pq.pop();

        searchLogger.debug("pq retrieved currnt Cell ID: [%s]", currentCellId.c_str());

        // Break the loop if the destination cell is reached
        if (currentCellId == destinationCellId) break;

        Cell &currentCellData = cells.at(currentCellId);
        searchLogger.debug("currnt Cell edge count: %d", currentCellData.edges.size());
        for (const auto &neighborEntry: currentCellData.edges) {
            const auto &neighborCellId = neighborEntry.first;
            searchLogger.debug("neighbour Cell ID: [%s]", neighborCellId.c_str());
            // Calculate the new distance from the source to the neighbor cell
            const uint64_t newDistance = distances[currentCellId] + cells[currentCellId].edges[neighborCellId];

            searchLogger.debug("known distance from source to   curr: %d", distances[currentCellId]);
            searchLogger.debug("known distance from source to  neigh: %d", distances[neighborCellId]);
            searchLogger.debug("known distance from currnt to  neigh: %d", cells[currentCellId].edges[neighborCellId]);

            searchLogger.debug("distance from source to  neigh: %d (new possible)", newDistance);

            // Update the distance and previous cell if the new distance is shorter
            if (newDistance < distances[neighborCellId]) {
                distances[neighborCellId] = newDistance;
                pq.push(make_pair(newDistance, neighborCellId));

                searchLogger.debug("new distance from source to  neigh: %d (updated)", newDistance);
                searchLogger.debug("new pq added entry <%d,[%s]>", newDistance, neighborCellId.c_str());
            }
        }
    }

    searchLogger.debug("Shortest distance: %d", distances[destinationCellId]);
    uint64_t shortestPath = distances[destinationCellId];

    locker.sharedUnlock();

    return shortestPath;
}
