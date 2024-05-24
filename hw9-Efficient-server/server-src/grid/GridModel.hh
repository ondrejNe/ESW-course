
#ifndef GRID_MODEL_HH
#define GRID_MODEL_HH

#include <unordered_map>
#include <map>
#include <set>
#include <cmath>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <queue>
#include <unordered_set>
#include <limits>
#include <cstdint>
#include <shared_mutex>
#include <string>
#include <future>
#include <utility>

#include "scheme.pb.h"

#include "ReentrantSharedLocker.hh"
#include "Logger.hh"
#include "ThreadPool.hh"

#define ACTIVE_LOGGER_API       true
#define ACTIVE_LOGGER_PROTO     false
#define ACTIVE_LOGGER_SEARCH    false

using namespace std;

/**
 * Represents a point in the grid
 */
struct Point {
    uint64_t x;
    uint64_t y;
};

/**
 * Represents a cell in the grid which is a collection of points
 * and edges to other cells.
 */
class Cell {
public:
    uint64_t    id;
    uint64_t    coordX;
    uint64_t    coordY;
    Point       point;
    unordered_map <uint64_t, uint64_t> edges;

    // Default constructor
    Cell() : id(0), coordX(0), coordY(0), point(Point{}) {
        edges.reserve(1000);
    }

    // Parameterized constructor
    Cell(uint64_t id, uint64_t x, uint64_t y, Point p) :
        id(id), coordX(x), coordY(y), point(p) {
        edges.reserve(1000);
    }
};

/**
 * Represents the grid data structure which is a collection of cells.
 */
class Grid {
private:
    // Graph structure
    unordered_map <uint64_t, Cell>                              cells;
    // Search structure
    // originCellId -> destinationCellId -> distance (adjacency list)
    unordered_map <uint64_t, unordered_map<uint64_t, uint64_t>> distances;

    // Logging
    PrefixedLogger  apiLogger;
    PrefixedLogger  protoLogger;
    PrefixedLogger  searchLogger;
    // Workers
    ThreadPool      &resourcePool;

    // Distance metric between points
    uint64_t euclideanDistance(Point &p1, Point &p2);

public:
    Grid(ThreadPool &resourcePool) :
            apiLogger("[GRID-API]", ACTIVE_LOGGER_API),
            protoLogger("[GRID-PROTO]", ACTIVE_LOGGER_PROTO),
            searchLogger("[GRIDSEARCH]", ACTIVE_LOGGER_SEARCH),
            resourcePool(resourcePool) {
        cells.reserve(10000);
    }

    /* Point API */
    void addPoint(Point &point, uint64_t &cellId);

    uint64_t getPointCellId(Point &point);

    void addEdge(uint64_t &originCellId, uint64_t &destinationCellId, uint64_t length);

    /* Grid API */
    void resetGrid();

    uint64_t dijkstra(uint64_t &originCellId, uint64_t &destinationCellId);

    uint64_t allDijkstra(uint64_t &originCellId);

    /* Input processing API */

    void processWalk(const esw::Walk &walk);

    uint64_t processOneToOne(const esw::OneToOne &oneToOne);

    uint64_t processOneToAll(const esw::OneToAll &oneToAll);

    void processReset(const esw::Reset &reset);
};

#endif //GRID_MODEL_HH
