#include "DataGrid.hh"

void DataGrid::processWalk(const esw::Walk& walk) {
    const auto& locations = walk.locations();
    const auto& lengths = walk.lengths();

    // Ensure there are at least two locations and lengths
    if (locations.size() >= 2 && lengths.size() >= 1) {
        for (int i = 0; i < locations.size() - 1; ++i) {
            const auto& location1 = locations.Get(i);
            const auto& location2 = locations.Get(i + 1);
            const auto& length = lengths.Get(i);

            Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
            Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

            addEdge(origin, destination, length);
        }
    }

    logContent();
}

uint64_t DataGrid::processOneToOne(const esw::OneToOne& oneToOne) {
    const auto& location1 = oneToOne.origin();
    const auto& location2 = oneToOne.destination();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

    return shortestPathDijkstra(origin, destination);
}

uint64_t DataGrid::processOneToAll(const esw::OneToAll& oneToAll) {
    const auto& location1 = oneToAll.origin();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};

    return allOtherShortestPath(origin);
}

void DataGrid::processReset(const esw::Reset& reset) {
    resetDataGrid();
}
