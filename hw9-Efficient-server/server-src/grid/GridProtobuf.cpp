
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
//#define PROTO_LOGGER
PrefixedLogger protoLogger = PrefixedLogger("[PROTOBUF  ]", true);
#define STATS_LOGGER
PrefixedLogger statsLogger = PrefixedLogger("[STATISTICS]", true);

// Class definition -------------------------------------------------------------------------------
void Grid::processWalk(const esw::Walk &walk) {
#ifdef PROTO_LOGGER
    protoLogger.debug("Processing Walk message");
#endif
    walk_count++;

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

    logGridGraph();
}

uint64_t Grid::processOneToOne(const esw::OneToOne &oneToOne) {
#ifdef PROTO_LOGGER
    protoLogger.info("Processing OneToOne message");
#endif
    oneToOne_count++;

    const auto &location1 = oneToOne.origin();
    const auto &location2 = oneToOne.destination();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    uint64_t originCellId = getPointCellId(origin);

    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};
    uint64_t destinationCellId = getPointCellId(destination);

    uint64_t shortestPath = dijkstra(originCellId, destinationCellId);
#ifdef STATS_LOGGER
    statsLogger.warn("Shortest path: %llu from: %llu to: %llu cells: %d edges: %d", shortestPath, originCellId,
                     destinationCellId, cells.size(), edges_count);
    statsLogger.warn("Walks: %d OneToOne: %d OneToAll: %d locations: %d", walk_count, oneToOne_count, oneToAll_count,
                     location_count);
#endif
    logGridGraph();
    return shortestPath;
}

uint64_t Grid::processOneToAll(const esw::OneToAll &oneToAll) {
#ifdef PROTO_LOGGER
    protoLogger.info("Processing OneToAll message");
#endif
    oneToAll_count++;

    const auto &location1 = oneToAll.origin();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    uint64_t originCellId = getPointCellId(origin);

    uint64_t shortestPath = allDijkstra(originCellId);
#ifdef STATS_LOGGER
    statsLogger.warn("Total distance: %llu to: %llu cells: %d edges: %d", shortestPath, originCellId, cells.size(),
                     edges_count);
    statsLogger.warn("Walks: %d OneToOne: %d OneToAll: %d locations: %d", walk_count, oneToOne_count, oneToAll_count,
                     location_count);
#endif
    logGridGraph();
    return shortestPath;
}

void Grid::processReset() {
    resetGrid();
}
