
#ifndef GRID_MODEL_HH
#define GRID_MODEL_HH

#include <unordered_map>
#include <map>
#include <set>
#include <cmath>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <limits>
#include <cstdint>
#include <shared_mutex>
#include <string>
#include <iostream>

#include "scheme.pb.h"

#include "Locker.hh"
#include "Logger.hh"

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
 * Represents a cell in the grid which is a collection of points
 * and edges to other cells.
 */
struct Cell {
    string                  id;
    uint64_t                coordX;
    uint64_t                coordY;
    set <Point>             points;
    map <string, uint64_t>  edges;
};

/**
 * Represents the grid data structure which is a collection of cells.
 */
class Grid {
public:
    Grid() : searchLogger("[GRID-DIJKSTRA] ", DEBUG), protoLogger("[GRID-PROTO]", DEBUG), apiLogger("[GRID-API]", DEBUG) {}

    /* Point API */
    void addPoint(Point &point, string &cellId);

    string getPointCellId(Point &point);

    string searchNeighbourCells(string &probableId, Point &point);

    void addEdge(Point &origin, Point &destination, uint64_t length);

    /* Grid API */
    void resetGrid();

    uint64_t dijkstra(Point &origin, Point &destination);

    uint64_t allDijkstra(Point &origin);
    
    /* Input processing API */
    
    void processWalk(const esw::Walk &walk);

    uint64_t processOneToOne(const esw::OneToOne &oneToOne);

    uint64_t processOneToAll(const esw::OneToAll &oneToAll);

    void processReset(const esw::Reset &reset);

private:
    // Basic data structures
    unordered_map <string, Cell> cells;
    // RW lock
    SharedLocker locker;
    // Logging
    PrefixedLogger searchLogger;
    PrefixedLogger protoLogger;
    PrefixedLogger apiLogger;

    // Point-based conversions
    string pointToCellId(Point &point);
    
    pair <uint64_t, uint64_t> pointToCellCoords(Point &point);
    
    string pointToKey(Point &point);

    Cell pointToCell(Point &point);

    // String-based conversions
    Point keyToPoint(string &key);

    // Value based conversions
    string valuesToCellId(uint64_t x, uint64_t y);

    Cell valuesToCell(uint64_t &x, uint64_t &y);

    // Distance metric between points
    uint64_t euclideanDistance(Point &p1, Point &p2);
};

#endif //GRID_MODEL_HH
