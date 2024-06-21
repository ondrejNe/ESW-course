
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
PrefixedLogger searchLogger = PrefixedLogger("[SEARCHING]", false);

// Class definition -------------------------------------------------------------------------------
uint64_t Grid::allDijkstra(uint64_t &originCellId) {
    uint64_t sum = 0;

    for (const auto &entry: cells) {
        uint64_t destinationCellId = entry.first;
        uint64_t shortestPath = dijkstra(originCellId, destinationCellId);
        if (shortestPath != numeric_limits<uint64_t>::max()) {
            sum += shortestPath;
        }
    }

    return sum;
}

uint64_t Grid::dijkstra(uint64_t &originCellId, uint64_t &destinationCellId) {
    searchLogger.info("Dijkstra from %llu to %llu", originCellId, destinationCellId);
    priority_queue<pair<uint64_t, uint64_t>, vector<pair<uint64_t, uint64_t>>, greater<>> pq;

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        uint64_t currentCellId = pq.top().second;
        pq.pop();
        searchLogger.info("PQ popped current cell %llu", currentCellId);

        // Break the loop if the destination cell is reached
        if (currentCellId == destinationCellId) break;

        // Iterate over the neighbors of the current cell if they exist
        auto edgeIt = edges.find(currentCellId);
        if (edgeIt == edges.end()) continue;

        for (const auto &neighborEntry: edgeIt->second) {
            const auto &neighborCellId = neighborEntry.first;
//            const auto &neighborLength = neighborEntry.second;
//            searchLogger.info("Found neighbor cell %llu with length %llu", neighborCellId, neighborLength);

            uint64_t originCurrent = distances[originCellId][currentCellId];
            if (originCurrent == 0) {
                searchLogger.error("Origin to current %llu is 0", currentCellId);
            }
            uint64_t currentNeighbor = distances[currentCellId][neighborCellId];
            searchLogger.info("Origin to current %llu", originCurrent);
            searchLogger.info("Current to neighbor %llu", currentNeighbor);

            const uint64_t possibleOriginNeighbor = originCurrent + currentNeighbor;
            searchLogger.info("Possible origin to neighbor %llu", possibleOriginNeighbor);

            uint64_t originNeighbor = distances[originCellId][neighborCellId];
            searchLogger.info("Origin to neighbor %llu", originNeighbor);

            if (originNeighbor == 0 || possibleOriginNeighbor <= originNeighbor) {
                distances[originCellId][neighborCellId] = possibleOriginNeighbor;
                pq.push(make_pair(possibleOriginNeighbor, neighborCellId));
                searchLogger.info("Origin to neighbor %llu updated to %llu", neighborCellId, possibleOriginNeighbor);
            }
        }
    }

    uint64_t shortestPath = distances[originCellId][destinationCellId];
    return shortestPath;
}
