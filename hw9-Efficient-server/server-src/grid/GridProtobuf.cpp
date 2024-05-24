
#include "GridModel.hh"

void Grid::processWalk(const esw::Walk &walk) {
    protoLogger.info("Processing walk message");
    const auto &locations = walk.locations();
    const auto &lengths = walk.lengths();

    if (locations.size() < 2 || lengths.size() < 1) return;

    auto &location1 = locations.Get(0);
    auto &location2 = locations.Get(1);
    auto &length = lengths.Get(0);

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

    uint64_t originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    uint64_t destinationCellId = getPointCellId(destination);
    addPoint(destination, destinationCellId);

    addEdge(originCellId, destinationCellId, length);

    for (int i = 1; i < locations.size() - 1; ++i) {
        auto &location = locations.Get(i + 1);
        auto &len = lengths.Get(i);

        destination = {static_cast<uint64_t>(location.x()), static_cast<uint64_t>(location.y())};

        originCellId = destinationCellId;
        destinationCellId = getPointCellId(destination);
        addPoint(destination, destinationCellId);
        addEdge(originCellId, destinationCellId, len);
    }

}

uint64_t Grid::processOneToOne(const esw::OneToOne &oneToOne) {
    const auto &location1 = oneToOne.origin();
    const auto &location2 = oneToOne.destination();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    uint64_t originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};
    uint64_t destinationCellId = getPointCellId(destination);
    addPoint(destination, destinationCellId);

    uint64_t shortestPath = dijkstra(originCellId, destinationCellId);
    apiLogger.warn("Shortest path: %llu", shortestPath);
    return shortestPath;
}

uint64_t Grid::processOneToAll(const esw::OneToAll &oneToAll) {
    const auto &location1 = oneToAll.origin();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    uint64_t originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    uint64_t shortestPath = allDijkstra(originCellId);
    apiLogger.warn("Total length: %llu", shortestPath);
    apiLogger.warn("Total cells: %d", cells.size());
    return shortestPath;
}

void Grid::processReset(const esw::Reset &reset) {
    resetGrid();
}
