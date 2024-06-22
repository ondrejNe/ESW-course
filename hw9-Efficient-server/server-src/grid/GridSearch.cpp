
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define SEARCH_LOGGER
PrefixedLogger searchLogger = PrefixedLogger("[SEARCHING]", true);
#define SEARCH_STATS_LOGGER
PrefixedLogger searchStatsLogger = PrefixedLogger("[SEARCH STATS]", true);

#ifdef SEARCH_STATS_LOGGER
uint64_t maxEdges = 0;
uint64_t maxPqSize = 0;
#endif

// Class definition -------------------------------------------------------------------------------
uint64_t Grid::allDijkstra(uint64_t &originCellId) {
    dijkstra(originCellId, originCellId, ONE_TO_ALL);

    uint64_t sum = 0;

    for (const auto &[destinationCellId, stats]: cells[originCellId].stats) {
        sum += stats.distance;
    }

    return sum;
}

uint64_t Grid::dijkstra(uint64_t &originCellId, uint64_t &destinationCellId, bool oneToAll) {
#ifdef SEARCH_LOGGER
    if (oneToAll) {
        searchLogger.debug("--- Dijkstra from %llu to all ---", originCellId);
    } else {
        searchLogger.debug("--- Dijkstra from %llu to %llu ---", originCellId, destinationCellId);
    }
#endif
    vec.clear();
    visited.clear();
    std::priority_queue <
    std::pair < uint64_t, uint64_t >,
            std::vector < std::pair < uint64_t, uint64_t >>,
            std::greater<>
            > pq(std::greater<>(), vec);
    tsl::robin_map<uint64_t, Stat> &originStats = cells[originCellId].stats;

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        uint64_t originCurrent = pq.top().first;
        uint64_t currentCellId = pq.top().second;
        tsl::robin_map<uint64_t, Stat> &currentStats = cells[currentCellId].stats;
        visited[currentCellId] = 1;
#ifdef SEARCH_LOGGER
        searchLogger.debug("PQ popped current %llu", currentCellId);
        searchLogger.debug("Origin to current distance %llu", originCurrent);
#endif
#ifdef SEARCH_STATS_LOGGER
        if (currentStats.size() > maxEdges) {
            maxEdges = currentStats.size();
        }
        if (pq.size() > maxPqSize) {
            maxPqSize = pq.size();
        }
#endif
        if (currentCellId == destinationCellId && !oneToAll) {
#ifdef SEARCH_LOGGER
            searchLogger.debug("Destination cell %llu reached with distance %llu", currentCellId, originCurrent);
#endif
            break;
        }

        pq.pop();

        for (const auto &[neighborCellId, stats]: currentStats) {
            if (stats.edge == 0) continue;

            uint64_t originNeighbor = originStats[neighborCellId].distance;
            const uint64_t possibleOriginNeighbor = originCurrent + stats.edge;
#ifdef SEARCH_LOGGER
            searchLogger.debug("Neighbor cell %llu", neighborCellId);
            searchLogger.debug("Current to neighbor edge %llu", stats.edge);
            searchLogger.debug("Origin to neighbor distance %llu", originNeighbor);
            searchLogger.debug("Possible new origin to neighbor distance %llu", possibleOriginNeighbor);
#endif
            if (originNeighbor == 0 || possibleOriginNeighbor < originNeighbor) {
#ifdef SEARCH_LOGGER
                searchLogger.debug("Existing origin to neighbor distance %llu update to %llu", originNeighbor,
                                   possibleOriginNeighbor);
#endif
                originNeighbor = possibleOriginNeighbor;
                originStats[neighborCellId].distance = originNeighbor;

                if (visited[neighborCellId] == 1) {
#ifdef SEARCH_LOGGER
                    searchLogger.debug("Neighbor cell %llu already visited", currentCellId);
#endif
                    continue;
                }
                pq.push(make_pair(originNeighbor, neighborCellId));
#ifdef SEARCH_LOGGER
                searchLogger.debug("Added neighbor %llu to PQ", neighborCellId);
#endif
            }
        }
    }
#ifdef SEARCH_STATS_LOGGER
    searchStatsLogger.info("Visited size: %lu", visited.size());
    searchStatsLogger.info("Max edges: %lu", maxEdges);
    searchStatsLogger.info("Max PQ size: %lu", maxPqSize);
#endif
    return originStats[destinationCellId].distance;
}
