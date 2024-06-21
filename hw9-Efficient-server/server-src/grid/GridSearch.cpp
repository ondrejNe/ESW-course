
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define SEARCH_LOGGER
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
#ifdef SEARCH_LOGGER
    searchLogger.debug("Dijkstra from %llu to %llu", originCellId, destinationCellId);
#endif
    priority_queue<pair<uint64_t, uint64_t>, vector<pair<uint64_t, uint64_t>>, greater<>> pq;
    tsl::robin_map <uint64_t, uint64_t> visited = tsl::robin_map<uint64_t, uint64_t>();

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        uint64_t currentCellId = pq.top().second;
        pq.pop();
#ifdef SEARCH_LOGGER
        searchLogger.debug("PQ popped current cell %llu", currentCellId);
#endif
        if (visited[currentCellId] == 1) {
#ifdef SEARCH_LOGGER
            searchLogger.debug("Current cell %llu already visited", currentCellId);
#endif
            continue;
        }
        visited[currentCellId] = 1;

        // Iterate over the neighbors of the current cell if they exist
        auto edgeIt = edges.find(currentCellId);
        if (edgeIt == edges.end()) continue;

        for (const auto &neighborEntry: edgeIt->second) {
            const auto &neighborCellId = neighborEntry.first;
            const auto &currentNeighbor = neighborEntry.second;
#ifdef SEARCH_LOGGER
            searchLogger.debug("Current to neighbor %llu", currentNeighbor);
#endif
            uint64_t originCurrent = distances[originCellId][currentCellId];
#ifdef SEARCH_LOGGER
            searchLogger.debug("Origin to current %llu", originCurrent);
#endif
            const uint64_t possibleOriginNeighbor = originCurrent + currentNeighbor;
#ifdef SEARCH_LOGGER
            searchLogger.debug("Possible origin to neighbor %llu", possibleOriginNeighbor);
#endif
            uint64_t originNeighbor = distances[originCellId][neighborCellId];
#ifdef SEARCH_LOGGER
            searchLogger.debug("Origin to neighbor %llu", originNeighbor);
#endif
            if (originNeighbor == 0 || possibleOriginNeighbor < originNeighbor) {
                distances[originCellId][neighborCellId] = possibleOriginNeighbor;
                pq.push(make_pair(possibleOriginNeighbor, neighborCellId));
#ifdef SEARCH_LOGGER
                searchLogger.debug("Origin to neighbor %llu updated to %llu", neighborCellId, possibleOriginNeighbor);
#endif
            }
        }
    }

    uint64_t shortestPath = distances[originCellId][destinationCellId];
    return shortestPath;
}
