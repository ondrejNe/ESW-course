
#include "GridModel.hh"

void Grid::processWalk(const esw::Walk &walk) {
    const auto &locations = walk.locations();
    const auto &lengths = walk.lengths();
    
    if (locations.size() < 2 || lengths.size() < 1) return;

    protoLogger.info("Walk message - %d locations - %d lengths", locations.size(), lengths.size());
//    vector<future<uint64_t>> futures;

    auto &location1 = locations.Get(0);
    auto &location2 = locations.Get(1);
    auto &length = lengths.Get(0);

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

    pair<uint64_t, uint64_t> originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    pair<uint64_t, uint64_t> destinationCellId = getPointCellId(destination);
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
//        futures.emplace_back(resourcePool.run([this, location1, location2, length]() -> uint64_t {

//            Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
//            Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};
//
//            string originCellId = getPointCellId(origin);
//            addPoint(origin, originCellId);
//
//            string destinationCellId = getPointCellId(destination);
//            addPoint(destination, destinationCellId);
//
//            addEdge(originCellId, destinationCellId, length);

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
    pair<uint64_t, uint64_t> originCellId = getPointCellId(origin);
        addPoint(origin, originCellId);

        Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};
    pair<uint64_t, uint64_t> destinationCellId = getPointCellId(destination);
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
    pair<uint64_t, uint64_t> originCellId = getPointCellId(origin);
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
