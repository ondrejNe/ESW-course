#include "DataGrid.hh"

// #define DEBUG_DJIKSTRA
// #define LOCKING

uint64_t DataGrid::shortestPathDijkstra(Point &origin, Point &destination) {
    #ifdef DEBUG_DJIKSTRA
    cout << "[DJIKSTRA]  from point (" << origin.x << "," << origin.y << ") to point (" << destination.x << "," << destination.y << ")" << endl;
    #endif

    // Find the cells where the points belong
    string originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    string destinationCellId = getPointCellId(destination);
    addPoint(destination, destinationCellId);

    #ifdef DEBUG_DJIKSTRA
    cout << "[DJIKSTRA]  from cell " << originCellId << " to cell " << destinationCellId << endl;
    #endif

    // Priority queue to store cells to be processed based on their distances
    priority_queue<pair<uint64_t, string>, vector<pair<uint64_t, string>>, greater<>> pq;
    // Map to store distances from the source to each cell
    unordered_map<string, uint64_t> distances;

    {
        #ifdef LOCKING
        shared_lock<shared_mutex> lock(rwMutex);
        #endif

        // Set all distances to infinity except for the source cell (set to 0)
        for (const auto& entry : cells) {
            distances[entry.first] = numeric_limits<uint64_t>::max();
        }
        distances[originCellId] = 0;

        // Add the source cell to the priority queue
        pq.push(make_pair(distances[originCellId], originCellId));
        #ifdef DEBUG_DJIKSTRA
        cout << "[DJIKSTRA]    pq added entry <" << distances[originCellId] << "," << originCellId << ">"<< endl;
        #endif

        // Main loop of Dijkstra's Algorithm
        while (!pq.empty()) {
            // Get the cell with the minimum distance from the priority queue
            string currentCellId = pq.top().second;
            pq.pop();
            #ifdef DEBUG_DJIKSTRA
            cout << "[DJIKSTRA]    pq retrieved currnt Cell ID: " << currentCellId << endl;
            #endif

            // Break the loop if the destination cell is reached
            if (currentCellId == destinationCellId) break;

            Cell& currentCellData = cells.at(currentCellId);

            for (const auto& neighborEntry : currentCellData.edges) {
                const auto& neighborCellId = neighborEntry.first;
                // Calculate the new distance from the source to the neighbor cell
                const uint64_t newDistance = distances[currentCellId] + cells[currentCellId].edges[neighborCellId];
                #ifdef DEBUG_DJIKSTRA
                cout << "[DJIKSTRA]      neighbour Cell ID: " << neighborCellId << endl;
                cout << "[DJIKSTRA]        distance from source to currnt: " << distances[currentCellId] << endl;
                cout << "[DJIKSTRA]        distance from source to  neigh: " << distances[neighborCellId] << endl;
                cout << "[DJIKSTRA]        distance from currnt to  neigh: " << cells[currentCellId].edges[neighborCellId] << endl;
                cout << "[DJIKSTRA]        distance from source to  neigh: " << newDistance << " (new possible)" << endl;
                #endif
                // Update the distance and previous cell if the new distance is shorter
                if (newDistance < distances[neighborCellId]) {
                    distances[neighborCellId] = newDistance;
                    pq.push(make_pair(newDistance, neighborCellId));
                    #ifdef DEBUG_DJIKSTRA
                    cout << "[DJIKSTRA]        distance from source to  neigh: " << newDistance << " (updated)" << endl;
                    cout << "[DJIKSTRA]    pq added entry <" << newDistance << "," << neighborCellId << ">"<< endl;
                    #endif
                }
            }
        }

        #ifdef LOCKING
        lock.unlock();
        #endif
    }

    #ifdef DEBUG_DJIKSTRA
    cout << "[DJIKSTRA]    distances: " << endl;
    for (const auto& entry : distances) {
        cout << "[DJIKSTRA]      Cell ID: " << entry.first << " Distance: " << entry.second << endl;
    }
    cout << "[DJIKSTRA]  Shortest distance: " << distances[destinationCellId] << endl;
    #endif

    return distances[destinationCellId];
}

uint64_t DataGrid::allOtherShortestPath(Point &origin) {
    // Find the cells where the points belong
    string originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    uint64_t sum = 0;

    vector<Point> examplePoints;

    {
        #ifdef LOCKING
        shared_lock<shared_mutex> lock(rwMutex);
        #endif

        for (const auto& entry : cells) {
            Point retrievedPoint = *entry.second.points.begin();
            examplePoints.push_back(retrievedPoint);
        }

        #ifdef LOCKING
        lock.unlock();
        #endif
    }

    for (Point examplePoint : examplePoints) {
        uint64_t shortestPath = shortestPathDijkstra(origin, examplePoint);
        if (shortestPath == numeric_limits<uint64_t>::max()) {
            #ifdef DEBUG_DJIKSTRA
            cout << "[DJIKSTRA]  shortest distance from: " << origin.x << "," << origin.y << " to: " << examplePoint.x << "," << examplePoint.y << " is: INFINITY" << endl;
            #endif
            continue;
        }
        sum += shortestPath;

        #ifdef DEBUG_DJIKSTRA
        cout << "[DJIKSTRA]  shortest distance from: " << origin.x << "," << origin.y << " to: " << examplePoint.x << "," << examplePoint.y << " is: " << shortestPath << endl;
        cout << "[DJIKSTRA]  sum: " << sum << endl;
        #endif
    }

    #ifdef DEBUG_DJIKSTRA
    cout << "[DJIKSTRA]  Total sum: " << sum << endl;
    #endif
    
    return sum;
}
