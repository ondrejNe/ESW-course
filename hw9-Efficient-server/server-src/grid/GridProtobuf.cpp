
#include "GridModel.hh"

void Grid::processWalk(const esw::Walk &walk) {
    const auto &locations = walk.locations();
    const auto &lengths = walk.lengths();

    if (locations.size() < 2 || lengths.size() < 1) return;

    protoLogger.info("Walk message - %d locations - %d lengths", locations.size(), lengths.size());

    auto &location1 = locations.Get(0);
    auto &location2 = locations.Get(1);
    auto &length = lengths.Get(0);

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};

    pair <uint64_t, uint64_t> originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    pair <uint64_t, uint64_t> destinationCellId = getPointCellId(destination);
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
    pair <uint64_t, uint64_t> originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    Point destination = {static_cast<uint64_t>(location2.x()), static_cast<uint64_t>(location2.y())};
    pair <uint64_t, uint64_t> destinationCellId = getPointCellId(destination);
    addPoint(destination, destinationCellId);

    return dijkstra(originCellId, destinationCellId);
}

uint64_t Grid::processOneToAll(const esw::OneToAll &oneToAll) {
    const auto &location1 = oneToAll.origin();

    Point origin = {static_cast<uint64_t>(location1.x()), static_cast<uint64_t>(location1.y())};
    pair <uint64_t, uint64_t> originCellId = getPointCellId(origin);
    addPoint(origin, originCellId);

    return allDijkstra(originCellId);
}

void Grid::processReset(const esw::Reset &reset) {
    UNUSED(reset);
    resetGrid();
}
