
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define SEARCH_LOGGER
PrefixedLogger searchLogger = PrefixedLogger("[SEARCHING]", true);
//#define SEARCH_STATS_LOGGER
PrefixedLogger searchStatsLogger = PrefixedLogger("[SEARCH STATS]", true);

#ifdef SEARCH_STATS_LOGGER
uint64_t maxEdges = 0;
uint64_t maxPqSize = 0;
#endif

// Class definition -------------------------------------------------------------------------------
uint64_t Grid::dijkstra(uint64_t &originCellId, uint64_t &destinationCellId, bool oneToAll) {
#ifdef SEARCH_LOGGER
    if (oneToAll) {
        searchLogger.debug("--- Dijkstra from %llu to all ---", originCellId);
    } else {
        searchLogger.debug("--- Dijkstra from %llu to %llu ---", originCellId, destinationCellId);
    }
#endif
    std::vector<std::pair<uint64_t, uint64_t>> vec = std::vector<std::pair<uint64_t, uint64_t>>();
    tsl::robin_map<uint64_t, uint64_t> visited = tsl::robin_map<uint64_t, uint64_t>();
    tsl::robin_map<uint64_t, uint64_t> distances = tsl::robin_map<uint64_t, uint64_t>();
    std::priority_queue <
    std::pair < uint64_t, uint64_t >,
            std::vector < std::pair < uint64_t, uint64_t >>,
            std::greater<>
            > pq(std::greater<>(), vec);
    tsl::robin_map<uint64_t, uint64_t> &originEdges = cells[originCellId].edges;

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        uint64_t originCurrent = pq.top().first;
        uint64_t currentCellId = pq.top().second;
        tsl::robin_map<uint64_t, uint64_t> &currentEdges = cells[currentCellId].edges;
        visited[currentCellId] = 1;
#ifdef SEARCH_LOGGER
        searchLogger.debug("PQ popped current %llu", currentCellId);
        searchLogger.debug("Origin to current distance %llu", originCurrent);
#endif
#ifdef SEARCH_STATS_LOGGER
        if (currentEdges.size() > maxEdges) {
            maxEdges = currentEdges.size();
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

        for (const auto &[neighborCellId, edge]: currentEdges) {
            if (edge == 0) continue;

            uint64_t originNeighbor = distances[neighborCellId];
            const uint64_t possibleOriginNeighbor = originCurrent + edge;
#ifdef SEARCH_LOGGER
            searchLogger.debug("Neighbor cell %llu", neighborCellId);
            searchLogger.debug("Current to neighbor edge %llu", edge);
            searchLogger.debug("Origin to neighbor distance %llu", originNeighbor);
            searchLogger.debug("Possible new origin to neighbor distance %llu", possibleOriginNeighbor);
#endif
            if (originNeighbor == 0 || possibleOriginNeighbor < originNeighbor) {
#ifdef SEARCH_LOGGER
                searchLogger.debug("Existing origin to neighbor distance %llu update to %llu", originNeighbor,
                                   possibleOriginNeighbor);
#endif
                originNeighbor = possibleOriginNeighbor;
                distances[neighborCellId] = originNeighbor;

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
    searchStatsLogger.info("Distances size: %lu", distances.size());
    searchStatsLogger.info("Max edges: %lu", maxEdges);
    searchStatsLogger.info("Max PQ size: %lu", maxPqSize);
#endif

    if (oneToAll) {
        uint64_t sum = 0;

        for (const auto &[destinationCellId, edge]: originEdges) {
            sum += edge;
        }

        return sum;
    } else {
        return distances[destinationCellId];
    }
}
