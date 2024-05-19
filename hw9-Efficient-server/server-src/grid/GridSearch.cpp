
#include "GridModel.hh"

uint64_t Grid::allDijkstra(string &originCellId) {
    uint64_t sum = 0;
    vector<future<uint64_t>> futures;

    searchLogger.info("All Dijkstra cell count: %d", cells.size());

    locker.sharedLock();
    for (const auto &entry: cells) {
        string id = entry.first;
        futures.emplace_back(resourcePool.run([this, originCellId, id]() -> uint64_t {
            string originId = originCellId;
            string destinationId = id;
            this->searchLogger.debug("Processing [%s] -> [%s]", originId.c_str(), destinationId.c_str());
            uint64_t shortestPath = this->dijkstra(originId, destinationId);
            if (shortestPath == numeric_limits<uint64_t>::max()) {
                return 0;
            }
            return shortestPath;
        }));
    }
    locker.sharedUnlock();

    for (auto &f : futures) {
        searchLogger.debug("Waiting for future");
        sum += f.get();
        searchLogger.debug("Partial sum: %d", sum);
    }

    return sum;
}

uint64_t Grid::dijkstra(string &originCellId, string &destinationCellId) {
    searchLogger.debug("Shortest path from [%s] to [%s]", originCellId.c_str(), destinationCellId.c_str());

    // Priority queue to store cells to be processed based on their distances
    priority_queue < pair < uint64_t, string >, vector < pair < uint64_t, string >>, greater<>> pq;

    distancesLocker.uniqueLock();
    if (distances.find(originCellId) == distances.end()) {
        distances[originCellId] = unordered_map<string, uint64_t>();
        distances[originCellId][originCellId] = 0;
    }
    distancesLocker.uniqueUnlock();

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        string currentCellId = pq.top().second;
        pq.pop();

        // Break the loop if the destination cell is reached
        if (currentCellId == destinationCellId) break;

        locker.sharedLock();
        Cell &currentCellData = cells.at(currentCellId);
        locker.sharedUnlock();

        currentCellData.locker.sharedLock();
        for (const auto &neighborEntry: currentCellData.edges) {
            const auto &neighborCellId = neighborEntry.first;
            const auto &neighborLength = neighborEntry.second;

            distancesLocker.uniqueLock();
            if (distances[originCellId].find(neighborCellId) == distances[originCellId].end()) {
                distances[originCellId][neighborCellId] = numeric_limits<uint64_t>::max();
            }
            uint64_t currentDistance = distances[originCellId][currentCellId];
            uint64_t neighborDistance = distances[originCellId][neighborCellId];
            distancesLocker.uniqueUnlock();

            // Calculate the new distance from the source to the neighbor cell
            const uint64_t newDistance = currentDistance + neighborLength;

            // Update the distance and previous cell if the new distance is shorter
            if (newDistance < neighborDistance) {
                distancesLocker.uniqueLock();
                distances[originCellId][neighborCellId] = newDistance;
                distancesLocker.uniqueUnlock();
                pq.push(make_pair(newDistance, neighborCellId));
            }
        }
        currentCellData.locker.sharedUnlock();
    }

    distancesLocker.sharedLock();
    uint64_t shortestPath = distances[originCellId][destinationCellId];
    distancesLocker.sharedUnlock();

    return shortestPath;
}
