
#include "GridModel.hh"

// Global variables -------------------------------------------------------------------------------
#define PROTO_STATS_LOGGER
//#define PROTO_PROCESS_LOGGER
PrefixedLogger protoLogger = PrefixedLogger("[PROTOBUF  ]", true);

std::shared_mutex rwLock;
// Class definition -------------------------------------------------------------------------------
void processWalk(GridData &gridData, GridStats &gridStats, const esw::Walk &walk) {
    rwLock.lock();
#ifdef PROTO_PROCESS_LOGGER
    protoLogger.debug("Processing Walk message");
#endif
    gridStats.walk_count++;

    const auto &locations = walk.locations();
    const auto &lengths = walk.lengths();

    if (locations.size() < 2 || lengths.size() < 1) {
        rwLock.unlock();
        return;
    }

    auto &location1 = locations.Get(0);
    auto &location2 = locations.Get(1);
    auto &length = lengths.Get(0);

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

    uint64_t originCellId = gridData.getPointCellId(origin);
    gridData.addPoint(gridStats, origin, originCellId);
    uint64_t destinationCellId = gridData.getPointCellId(destination);
    gridData.addPoint(gridStats, destination, destinationCellId);
    gridData.addEdge(gridStats, originCellId, destinationCellId, length);

    for (int i = 1; i < locations.size() - 1; ++i) {
        auto &location = locations.Get(i + 1);
        auto &len = lengths.Get(i);

        destination = {static_cast<uint64_t>(location.x()), static_cast<uint64_t>(location.y())};

        originCellId = destinationCellId;
        destinationCellId = gridData.getPointCellId(destination);
        gridData.addPoint(gridStats, destination, destinationCellId);
        gridData.addEdge(gridStats, originCellId, destinationCellId, len);
    }
#ifdef PROTO_PROCESS_LOGGER
    protoLogger.debug("Processed Walk message");
#endif
    rwLock.unlock();
}

uint64_t processOneToOne(GridData &gridData, GridStats &gridStats, const esw::OneToOne &oneToOne) {
    rwLock.lock_shared();
#ifdef PROTO_PROCESS_LOGGER
    protoLogger.info("Processing OneToOne message");
#endif
    gridStats.oneToOne_count++;

    const auto &location1 = oneToOne.origin();
    const auto &location2 = oneToOne.destination();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    uint64_t originCellId = gridData.getPointCellId(origin);

    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};
    uint64_t destinationCellId = gridData.getPointCellId(destination);

    uint64_t shortestPath = dijkstra(gridData, originCellId, destinationCellId, ONE_TO_ONE);
#ifdef PROTO_STATS_LOGGER
    protoLogger.warn("Shortest path: %llu from: %llu to: %llu", shortestPath, originCellId, destinationCellId);
#endif
#ifdef PROTO_PROCESS_LOGGER
    protoLogger.info("Processed OneToOne message");
#endif
//    gridData.logGridGraph();
//    gridStats.logGridStats();
    rwLock.unlock_shared();
    return shortestPath;
}

uint64_t processOneToAll(GridData &gridData, GridStats &gridStats, const esw::OneToAll &oneToAll) {
    rwLock.lock_shared();
#ifdef PROTO_PROCESS_LOGGER
    protoLogger.info("Processing OneToAll message");
#endif
    gridStats.oneToAll_count++;

    const auto &location1 = oneToAll.origin();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    uint64_t originCellId = gridData.getPointCellId(origin);

    uint64_t shortestPath = dijkstra(gridData, originCellId, originCellId, ONE_TO_ALL);
#ifdef PROTO_STATS_LOGGER
    protoLogger.warn("Total path: %llu from: %llu", shortestPath, originCellId);
#endif
#ifdef PROTO_PROCESS_LOGGER
    protoLogger.info("Processed OneToAll message");
#endif
    gridData.logGridGraph();
    gridStats.logGridStats();
    rwLock.unlock_shared();
    return shortestPath;
}

void processReset(GridData &gridData, GridStats &gridStats) {
    rwLock.lock();
    gridData.resetGrid(gridStats);
    rwLock.unlock();
}
