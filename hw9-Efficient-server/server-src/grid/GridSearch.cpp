
#include "GridModel.hh"

uint64_t Grid::allDijkstra(pair <uint64_t, uint64_t> &originCellId) {
    uint64_t sum = 0;

    searchLogger.info("All Dijkstra cell count: %d", cells.size());

    for (const auto &entry: cells) {
        pair <uint64_t, uint64_t> id = entry.first;
        pair <uint64_t, uint64_t> originId = originCellId;
        pair <uint64_t, uint64_t> destinationId = id;
        uint64_t shortestPath = this->dijkstra(originId, destinationId);
        if (shortestPath == numeric_limits<uint64_t>::max()) {
            sum += 0;
        } else {
            sum += shortestPath;
        }
    }

    return sum;
}

uint64_t Grid::dijkstra(pair <uint64_t, uint64_t> &originCellId, pair <uint64_t, uint64_t> &destinationCellId) {

    // Priority queue to store cells to be processed based on their distances
    priority_queue < pair < uint64_t, pair < uint64_t, uint64_t > >, vector < pair < uint64_t, pair < uint64_t,
            uint64_t > >>, greater<>>
    pq;

    if (distances.find(originCellId) == distances.end()) {
        distances[originCellId] = unordered_map < pair < uint64_t, uint64_t >, uint64_t, PairHash > ();
        distances[originCellId][originCellId] = 0;
    }

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        pair <uint64_t, uint64_t> currentCellId = pq.top().second;
        pq.pop();

        // Break the loop if the destination cell is reached
        if (currentCellId == destinationCellId) break;

        Cell &currentCellData = cells.at(currentCellId);

        for (const auto &neighborEntry: currentCellData.edges) {
            const auto &neighborCellId = neighborEntry.first;
            const auto &neighborLength = neighborEntry.second;

            if (distances[originCellId].find(neighborCellId) == distances[originCellId].end()) {
                distances[originCellId][neighborCellId] = numeric_limits<uint64_t>::max();
            }
            uint64_t currentDistance = distances[originCellId][currentCellId];
            uint64_t neighborDistance = distances[originCellId][neighborCellId];

            // Calculate the new distance from the source to the neighbor cell
            const uint64_t newDistance = currentDistance + neighborLength;

            // Update the distance and previous cell if the new distance is shorter
            if (newDistance < neighborDistance) {
                distances[originCellId][neighborCellId] = newDistance;
                pq.push(make_pair(newDistance, neighborCellId));
            }
        }
    }

    uint64_t shortestPath = distances[originCellId][destinationCellId];

    return shortestPath;
}
