
#include "GridModel.hh"

void Grid::processWalk(const esw::Walk &walk) {
    const auto &locations = walk.locations();
    const auto &lengths = walk.lengths();
    
    if (locations.size() < 2 || lengths.size() < 1) return;

    protoLogger.debug("Processing walk with %d locations and %d lengths", locations.size(), lengths.size());

    for (int i = 0; i < locations.size() - 1; ++i) {
        const auto &location1 = locations.Get(i);
        const auto &location2 = locations.Get(i + 1);
        const auto &length = lengths.Get(i);

        Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
        Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

        addEdge(origin, destination, length);
    }
}

uint64_t Grid::processOneToOne(const esw::OneToOne &oneToOne) {
    const auto &location1 = oneToOne.origin();
    const auto &location2 = oneToOne.destination();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

    return dijkstra(origin, destination);
}

uint64_t Grid::processOneToAll(const esw::OneToAll &oneToAll) {
    const auto &location1 = oneToAll.origin();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};

    return allDijkstra(origin);
}

void Grid::processReset(const esw::Reset &reset) {
    UNUSED(reset);
    resetGrid();
}