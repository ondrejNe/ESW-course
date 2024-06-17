
#include "GridModel.hh"

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
    cout << "Dijkstra's Algorithm" << endl;
    priority_queue<pair<uint64_t, uint64_t>, vector<pair<uint64_t, uint64_t>>, greater<>> pq;

    if (distances.find(originCellId) == distances.end()) {
        distances[originCellId] = unordered_map<uint64_t, uint64_t>();
        distances[originCellId][originCellId] = 0;
    }

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    uint64_t pointX = cells[destinationCellId].pointX;
    uint64_t pointY = cells[destinationCellId].pointY;

    searchLogger.debug("Dijkstra's Algorithm from %llu to %llu", originCellId, destinationCellId);
    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        uint64_t currentCellId = pq.top().second;
        pq.pop();

        // Break the loop if the destination cell is reached
        if (currentCellId == destinationCellId) break;

        auto edgeIt = edges.find(currentCellId);
        if (edgeIt == edges.end()) continue;

        for (const auto &neighborEntry: edgeIt->second) {
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

                uint64_t neighborPointX = cells[neighborCellId].pointX;
                uint64_t neighborPointY = cells[neighborCellId].pointY;

                uint64_t dx = 0;
                if (pointX > neighborPointX) {
                    dx = pointX - neighborPointX;
                } else {
                    dx = neighborPointX - pointX;
                }
                uint64_t dy = 0;
                if (pointY > neighborPointY) {
                    dy = pointY - neighborPointY;
                } else {
                    dy = neighborPointY - pointY;
                }
                uint64_t euclidean = dx * dx + dy * dy;
                pq.push(make_pair(newDistance + euclidean, neighborCellId));
            }
        }
    }

    uint64_t shortestPath = distances[originCellId][destinationCellId];
    searchLogger.debug("Shortest path: %llu", shortestPath);
    cout << "Shortest path: " << shortestPath << endl;
    return shortestPath;
}
