
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
PrefixedLogger searchLogger = PrefixedLogger("[SEARCHING]", true);

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
    searchLogger.debug("Dijkstra from %llu to %llu", originCellId, destinationCellId);
    priority_queue<pair<uint64_t, uint64_t>, vector<pair<uint64_t, uint64_t>>, greater<>> pq;
    tsl::robin_map <uint64_t, uint64_t> visited = tsl::robin_map<uint64_t, uint64_t>();

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        uint64_t currentCellId = pq.top().second;
        pq.pop();
        searchLogger.debug("PQ popped current cell %llu", currentCellId);
        if (visited[currentCellId] == 1) {
            searchLogger.debug("Current cell %llu already visited", currentCellId);
            continue;
        }
        visited[currentCellId] = 1;

        // Iterate over the neighbors of the current cell if they exist
        auto edgeIt = edges.find(currentCellId);
        if (edgeIt == edges.end()) continue;

        for (const auto &neighborEntry: edgeIt->second) {
            const auto &neighborCellId = neighborEntry.first;
            const auto &currentNeighbor = neighborEntry.second;
            searchLogger.debug("Current to neighbor %llu", currentNeighbor);

            uint64_t originCurrent = distances[originCellId][currentCellId];
            searchLogger.debug("Origin to current %llu", originCurrent);

            const uint64_t possibleOriginNeighbor = originCurrent + currentNeighbor;
            searchLogger.debug("Possible origin to neighbor %llu", possibleOriginNeighbor);

            uint64_t originNeighbor = distances[originCellId][neighborCellId];
            searchLogger.debug("Origin to neighbor %llu", originNeighbor);

            if (originNeighbor == 0 || possibleOriginNeighbor < originNeighbor) {
                distances[originCellId][neighborCellId] = possibleOriginNeighbor;
                pq.push(make_pair(possibleOriginNeighbor, neighborCellId));
                searchLogger.debug("Origin to neighbor %llu updated to %llu", neighborCellId, possibleOriginNeighbor);
            }
        }
    }

    uint64_t shortestPath = distances[originCellId][destinationCellId];
    return shortestPath;
}
