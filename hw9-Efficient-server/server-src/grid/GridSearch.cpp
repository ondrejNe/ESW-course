
#include "GridModel.hh"

uint64_t Grid::allDijkstra(pair<uint64_t, uint64_t> &originCellId) {
    uint64_t sum = 0;
//    vector<future<uint64_t>> futures;

    searchLogger.info("All Dijkstra cell count: %d", cells.size());

//    locker.sharedLock();
    for (const auto &entry: cells) {
        pair<uint64_t, uint64_t> id = entry.first;
//        futures.emplace_back(resourcePool.run([this, originCellId, id]() -> uint64_t {
        pair<uint64_t, uint64_t> originId = originCellId;
        pair<uint64_t, uint64_t> destinationId = id;
            uint64_t shortestPath = this->dijkstra(originId, destinationId);
            if (shortestPath == numeric_limits<uint64_t>::max()) {
                sum += 0;
            } else {
                sum += shortestPath;
            }
//            return shortestPath;
//        }));
    }
//    locker.sharedUnlock();

//    for (auto &f : futures) {
//        searchLogger.debug("Waiting for future");
//        sum += f.get();
//        searchLogger.debug("Partial sum: %d", sum);
//    }

    return sum;
}

uint64_t Grid::dijkstra(pair<uint64_t, uint64_t> &originCellId, pair<uint64_t, uint64_t> &destinationCellId) {

    // Priority queue to store cells to be processed based on their distances
    priority_queue < pair < uint64_t, pair<uint64_t, uint64_t> >, vector < pair < uint64_t, pair<uint64_t, uint64_t> >>, greater<>> pq;

//    distancesLocker.uniqueLock();
    if (distances.find(originCellId) == distances.end()) {
        distances[originCellId] = unordered_map<pair<uint64_t, uint64_t>, uint64_t, PairHash>();
        distances[originCellId][originCellId] = 0;
    }
//    distancesLocker.uniqueUnlock();

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        pair<uint64_t, uint64_t> currentCellId = pq.top().second;
        pq.pop();

        // Break the loop if the destination cell is reached
        if (currentCellId == destinationCellId) break;

//        locker.sharedLock();
        Cell &currentCellData = cells.at(currentCellId);
//        locker.sharedUnlock();

//        currentCellData.locker.sharedLock();
        for (const auto &neighborEntry: currentCellData.edges) {
            const auto &neighborCellId = neighborEntry.first;
            const auto &neighborLength = neighborEntry.second;

//            distancesLocker.uniqueLock();
            if (distances[originCellId].find(neighborCellId) == distances[originCellId].end()) {
                distances[originCellId][neighborCellId] = numeric_limits<uint64_t>::max();
            }
            uint64_t currentDistance = distances[originCellId][currentCellId];
            uint64_t neighborDistance = distances[originCellId][neighborCellId];
//            distancesLocker.uniqueUnlock();

            // Calculate the new distance from the source to the neighbor cell
            const uint64_t newDistance = currentDistance + neighborLength;

            // Update the distance and previous cell if the new distance is shorter
            if (newDistance < neighborDistance) {
//                distancesLocker.uniqueLock();
                distances[originCellId][neighborCellId] = newDistance;
//                distancesLocker.uniqueUnlock();
                pq.push(make_pair(newDistance, neighborCellId));
            }
        }
//        currentCellData.locker.sharedUnlock();
    }

//    distancesLocker.sharedLock();
    uint64_t shortestPath = distances[originCellId][destinationCellId];
//    distancesLocker.sharedUnlock();

    return shortestPath;
}
