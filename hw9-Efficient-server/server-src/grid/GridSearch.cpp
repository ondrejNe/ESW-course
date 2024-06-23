
#include "GridModel.hh"
#include <chrono>
#include <boost/heap/priority_queue.hpp>

// Global variables -------------------------------------------------------------------------------
//#define SEARCH_STATS_LOGGER
//#define SEARCH_ALGO_LOGGER
#define SEARCH_TIME_LOGGER
PrefixedLogger searchLogger = PrefixedLogger("[SEARCHING ]", true);

// Class definition -------------------------------------------------------------------------------
uint64_t dijkstra(GridData &gridData, uint64_t &originCellId, uint64_t &destinationCellId, bool oneToAll) {
#ifdef SEARCH_TIME_LOGGER
    auto start = std::chrono::high_resolution_clock::now();
#endif
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

    ankerl::unordered_dense::map<uint64_t, uint64_t> visited;
    visited.reserve(115000);

    boost::heap::priority_queue<
            pair<uint64_t, uint64_t>,
            boost::heap::compare<greater<pair<uint64_t, uint64_t>>>
    > pq;
    pq.reserve(300);

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
        if (gridData.cells[currentCellId % CHUNKS][currentCellId].edges.size() > maxEdges) {
            maxEdges = gridData.cells[currentCellId % CHUNKS][currentCellId].edges.size();
        }
#endif
        for (const auto &[neighborCellId, edge, samples]: gridData.cells[currentCellId % CHUNKS][currentCellId].edges) {
            if (visited[neighborCellId] == 1) continue;
            pq.push({originCurrent + (edge / samples), neighborCellId});
        }
    }

#ifdef SEARCH_STATS_LOGGER
    searchLogger.warn("Max sums: %lu Max edges: %lu Max PQ size: %lu", maxSums, maxEdges, maxPqSize);
#endif
#ifdef SEARCH_TIME_LOGGER
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    searchLogger.debug("Function took %llu milliseconds to execute.", duration.count());
#endif
    return sum;
}
