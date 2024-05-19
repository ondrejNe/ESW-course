
#include "GridModel.hh"

void Grid::processWalk(const esw::Walk &walk) {
    const auto &locations = walk.locations();
    const auto &lengths = walk.lengths();
    
    if (locations.size() < 2 || lengths.size() < 1) return;

    protoLogger.info("Walk message - %d locations - %d lengths", locations.size(), lengths.size());
//    vector<future<uint64_t>> futures;

    for (int i = 0; i < locations.size() - 1; ++i) {
        const auto &location1 = locations.Get(i);
        const auto &location2 = locations.Get(i + 1);
        const auto &length = lengths.Get(i);

//        futures.emplace_back(resourcePool.run([this, location1, location2, length]() -> uint64_t {

            Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
            Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

            string originCellId = getPointCellId(origin);
            addPoint(origin, originCellId);

            string destinationCellId = getPointCellId(destination);
            addPoint(destination, destinationCellId);

            addEdge(originCellId, destinationCellId, length);

//            return 0;
//        }));
    }

//    for (auto &f : futures) {
//        f.get();
//    }
}

uint64_t Grid::processOneToOne(const esw::OneToOne &oneToOne) {
    const auto &location1 = oneToOne.origin();
    const auto &location2 = oneToOne.destination();

//    vector<future<uint64_t>> futures;
//
//    futures.emplace_back(resourcePool.run([this, location1, location2]() -> uint64_t {
        Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
        string originCellId = getPointCellId(origin);
        addPoint(origin, originCellId);

        Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};
        string destinationCellId = getPointCellId(destination);
        addPoint(destination, destinationCellId);

        return dijkstra(originCellId, destinationCellId);
//    }));

//    for (auto &f : futures) {
//        return f.get();
//    }
}

uint64_t Grid::processOneToAll(const esw::OneToAll &oneToAll) {
    const auto &location1 = oneToAll.origin();

//    vector<future<uint64_t>> futures;

//    futures.emplace_back(resourcePool.run([this, location1]() -> uint64_t {
        Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
        string originCellId = getPointCellId(origin);
        addPoint(origin, originCellId);

        return allDijkstra(originCellId);
//    }));
//
//    for (auto &f : futures) {
//        return f.get();
//    }
}

void Grid::processReset(const esw::Reset &reset) {
    UNUSED(reset);
    resetGrid();
}
