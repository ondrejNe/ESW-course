
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define SEARCH_STATS_LOGGER
//#define SEARCH_ALGO_LOGGER
PrefixedLogger searchLogger = PrefixedLogger("[SEARCHING ]", true);

// Class definition -------------------------------------------------------------------------------
uint64_t dijkstra(GridData &gridData, uint64_t &originCellId, uint64_t &destinationCellId, bool oneToAll) {
#ifdef SEARCH_ALGO_LOGGER
    if (oneToAll) {
        searchLogger.debug("--- Dijkstra from %llu to all ---", originCellId);
    } else {
        searchLogger.debug("--- Dijkstra from %llu to %llu ---", originCellId, destinationCellId);
    }
#endif
#ifdef SEARCH_STATS_LOGGER
    uint64_t maxSums = 0;
    uint64_t maxEdges = 0;
    uint64_t maxPqSize = 0;
#endif
    std::vector <std::pair<uint64_t, uint64_t>> vec;
    vec.reserve(300);
    tsl::robin_map<uint64_t, uint64_t,
    std::hash < uint64_t>, std::equal_to <uint64_t>, std::allocator <pair<uint64_t, uint64_t>>, true> visited;
    visited.reserve(115000);

    std::priority_queue <
    std::pair < uint64_t, uint64_t >,
            std::vector < std::pair < uint64_t, uint64_t >>,
            std::greater<>
            > pq(std::greater<>(), vec);

    uint64_t sum = 0;

    // Add the source cell to the priority queue
    pq.push({0, originCellId});

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
#ifdef SEARCH_STATS_LOGGER
        if (pq.size() > maxPqSize) {
            maxPqSize = pq.size();
        }
#endif
        // Get the cell with the minimum distance from the priority queue
        uint64_t originCurrent = pq.top().first;
        uint64_t currentCellId = pq.top().second;
        pq.pop();

        if (visited[currentCellId] == 1) continue;
        visited[currentCellId] = 1;

        if (currentCellId == destinationCellId && !oneToAll) {
            sum = originCurrent;
            break;
        } else {
            sum += originCurrent;
#ifdef SEARCH_STATS_LOGGER
            maxSums++;
#endif
        }

#ifdef SEARCH_STATS_LOGGER
        if (gridData.cells[currentCellId].edges.size() > maxEdges) {
            maxEdges = gridData.cells[currentCellId].edges.size();
        }
#endif
        for (const auto &[neighborCellId, edge]: gridData.cells[currentCellId].edges) {
            if (visited[neighborCellId] == 1) continue;
            pq.push({originCurrent + (edge.length / edge.samples), neighborCellId});
        }
    }

#ifdef SEARCH_STATS_LOGGER
    searchLogger.warn("Max sums: %lu Max edges: %lu Max PQ size: %lu", maxSums, maxEdges, maxPqSize);
#endif
    return sum;
}
