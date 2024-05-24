
#include "GridModel.hh"

template <class T>
class reservable_priority_queue: public std::priority_queue<T>
{
public:
    typedef typename std::priority_queue<T>::size_type size_type;
    reservable_priority_queue(size_type capacity = 0) { reserve(capacity); };
    void reserve(size_type capacity) { this->c.reserve(capacity); }
    size_type capacity() const { return this->c.capacity(); }
};

uint64_t Grid::allDijkstra(uint64_t &originCellId) {
    uint64_t sum = 0;

    searchLogger.info("All Dijkstra cell count: %d", cells.size());

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
    reservable_priority_queue <pair<uint64_t, uint64_t>> pq;
    pq.reserve(1000);

    if (distances.find(originCellId) == distances.end()) {
        distances[originCellId] = unordered_map<uint64_t, uint64_t>();
        distances[originCellId][originCellId] = 0;
    }

    // Add the source cell to the priority queue
    pq.push(make_pair(0, originCellId));

    // Main loop of Dijkstra's Algorithm
    while (!pq.empty()) {
        // Get the cell with the minimum distance from the priority queue
        uint64_t currentCellId = pq.top().second;
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
