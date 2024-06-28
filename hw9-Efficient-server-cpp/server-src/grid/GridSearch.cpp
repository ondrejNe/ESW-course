
#include "GridModel.hh"
#include <chrono>

// Global variables -------------------------------------------------------------------------------
//#define SEARCH_STATS_LOGGER
//#define SEARCH_ALGO_LOGGER
//#define SEARCH_TIME_LOGGER
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

    uint64_t heuristicSkips = 0;
#endif
    std::vector <std::pair<uint64_t, uint64_t>> vec;
    vec.reserve(300);
    ankerl::unordered_dense::map<uint64_t, uint64_t> visited;
    visited.reserve(115000);
    ankerl::unordered_dense::map<uint64_t, uint64_t> expandable;
    expandable.reserve(55000);

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
        if (gridData.cells[currentCellId % CHUNKS][currentCellId].edges.size() > maxEdges) {
            maxEdges = gridData.cells[currentCellId % CHUNKS][currentCellId].edges.size();
        }
#endif

        for (const auto &[neighborCellId, edge, samples]: gridData.cells[currentCellId % CHUNKS][currentCellId].edges) {
            if (visited[neighborCellId] == 1) continue;

            uint64_t inEdges = gridData.cells[neighborCellId % CHUNKS][neighborCellId].inEdges.size();
            uint64_t outEdges = gridData.cells[neighborCellId % CHUNKS][neighborCellId].edges.size();

            uint64_t id = neighborCellId;
            uint64_t dist = originCurrent + (edge / samples);

//            if (oneToAll) {
//                while (inEdges == 1 && outEdges == 1) {
//#ifdef SEARCH_STATS_LOGGER
//                    heuristicSkips++;
//#endif
//                    if (expandable[id] == 1) {
//                        searchLogger.warn("Cell %llu expanded from base %llu", id, currentCellId);
//                        searchLogger.warn("Type in %lu, out %lu", gridData.cells[currentCellId % CHUNKS][currentCellId].inEdges.size(),
//                                          gridData.cells[currentCellId % CHUNKS][currentCellId].edges.size());
//                    }
//                    expandable[id] = expandable[id] + 1;
//                    sum += dist;
//
//                    auto &[nextId, nextEdge, nextSamples] = gridData.cells[id % CHUNKS][id].edges.front();
//                    inEdges = gridData.cells[nextId % CHUNKS][nextId].inEdges.size();
//                    outEdges = gridData.cells[nextId % CHUNKS][nextId].edges.size();
//
//                    dist += (nextEdge / nextSamples);
//                    id = nextId;
//                }
//            }

            pq.push({dist, id});
        }
    }

#ifdef SEARCH_STATS_LOGGER
    searchLogger.warn("Max sums: %lu Max edges: %lu Max PQ size: %lu", maxSums, maxEdges, maxPqSize);
    searchLogger.warn("Heuristic skips: %lu", heuristicSkips);
    searchLogger.warn("Expanded cells: %lu", expandable.size());
    for (const auto &[id, count]: expandable) {
        if (count > 1) {
            searchLogger.warn("Cell %llu expanded %llu times", id, count);

        }
    }
#endif
#ifdef SEARCH_TIME_LOGGER
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    searchLogger.debug("Function took %llu milliseconds to execute.", duration.count());
#endif
    return sum;
}
