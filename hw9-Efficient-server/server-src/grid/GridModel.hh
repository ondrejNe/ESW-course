
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

#define UNUSED(x) (void)(x)

using namespace std;

/**
 * Represents a point in the grid
 */
struct Point {
    uint64_t x;
    uint64_t y;

    // Comparator
    bool operator<(const Point &other) const {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
};

/**
 * Hash function for pair<uint64_t, uint64_t>
 */
struct PairHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

/**
 * Represents a cell in the grid which is a collection of points
 * and edges to other cells.
 */
class Cell {
public:
    pair<uint64_t, uint64_t>                  id;
    uint64_t                coordX;
    uint64_t                coordY;
    set <Point>             points;
    unordered_map <pair<uint64_t, uint64_t>, uint64_t, PairHash>  edges;
    ReentrantSharedLocker   locker;

    Cell() : coordX(0), coordY(0) {}

    // Parameterized constructor
    Cell(const pair<uint64_t, uint64_t>& id, uint64_t x, uint64_t y)
            : id(id), coordX(x), coordY(y) {}

    // Copy constructor
    Cell(const Cell& other)
            : id(other.id), coordX(other.coordX), coordY(other.coordY),
              points(other.points), edges(other.edges) {
        // Note: locker is not copied
    }

    // Copy assignment operator
    Cell& operator=(const Cell& other) {
        if (this != &other) {
            id = other.id;
            coordX = other.coordX;
            coordY = other.coordY;
            points = other.points;
            edges = other.edges;
            // Note: locker is not copied
        }
        return *this;
    }
};

/**
 * Represents the grid data structure which is a collection of cells.
 */
class Grid
{
private:
    // Basic data structures
    unordered_map<pair<uint64_t, uint64_t>, Cell, PairHash> cells;
    unordered_map<pair<uint64_t, uint64_t>, unordered_map<pair<uint64_t, uint64_t>, uint64_t, PairHash>, PairHash> distances;
    ReentrantSharedLocker                                       distancesLocker;
    ReentrantSharedLocker                                       locker;
    // Logging
    PrefixedLogger                  searchLogger;
    PrefixedLogger                  protoLogger;
    PrefixedLogger                  apiLogger;
    // Workers
    ThreadPool                      &resourcePool;

    // Point-based conversions
    pair <uint64_t, uint64_t> pointToCellCoords(Point &point);

    Cell pointToCell(Point &point);

    // Value based conversions
    pair<uint64_t, uint64_t> valuesToCellId(uint64_t x, uint64_t y);

    // Distance metric between points
    uint64_t euclideanDistance(Point &p1, Point &p2);
public:
    Grid(ThreadPool &resourcePool) :
        searchLogger("[GRIDSEARCH]", DEBUG),
        protoLogger("[GRID-PROTO]", DEBUG), apiLogger("[GRID-API]", DEBUG),
        resourcePool(resourcePool) {}

    /* Point API */
    void addPoint(Point &point, pair<uint64_t, uint64_t> &cellId);

    pair<uint64_t, uint64_t> getPointCellId(Point &point);

    void addEdge(pair<uint64_t, uint64_t> &originCellId, pair<uint64_t, uint64_t> &destinationCellId, uint64_t length);

    /* Grid API */
    void resetGrid();

    uint64_t dijkstra(pair<uint64_t, uint64_t> &originCellId, pair<uint64_t, uint64_t> &destinationCellId);

    uint64_t allDijkstra(pair<uint64_t, uint64_t> &originCellId);
    
    /* Input processing API */
    
    void processWalk(const esw::Walk &walk);

    uint64_t processOneToOne(const esw::OneToOne &oneToOne);

    uint64_t processOneToAll(const esw::OneToAll &oneToAll);

    void processReset(const esw::Reset &reset);
};

#endif //GRID_MODEL_HH
