#include <unordered_map>
#include <map>
#include <string>
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

#include "protobuf/scheme.pb.h"

#ifndef DATAGRID_HH
#define DATAGRID_HH

using namespace std;

struct Point {
    uint64_t x;
    uint64_t y;

    bool operator<(const Point& other) const {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
};

struct Cell {
    uint64_t coordX;
    uint64_t coordY;
    // Name of the cell as x,y
    string id;
    set<Point> points;
    map<string, uint64_t> edges;
};

class DataGrid {
public:
    /* @file DataGridInsertion.cpp */
    void addPoint(Point &point, string &cellId);
    string getPointCellId(Point &point);
    string searchNeighbourCells(string &probableId, Point &point);
    void addEdge(Point &origin, Point &destination, uint64_t length);
    void resetDataGrid();
    /* @file DataGridLogging.cpp */
    void logContent();
    /* @file DataGridPath.cpp */
    uint64_t shortestPathDijkstra(Point &origin, Point &destination);
    uint64_t allOtherShortestPath(Point &origin);
    /* @file DataGridProtobuf.cpp */
    void processWalk(const esw::Walk& walk);
    uint64_t processOneToOne(const esw::OneToOne& oneToOne);
    uint64_t processOneToAll(const esw::OneToAll& oneToAll);
    void processReset(const esw::Reset& reset);
private:
    // Basic data structures - main data storing structure
    unordered_map<string, Cell> cells;
    // Read-write lock
    shared_mutex rwMutex;
    /* @file DataGridUtil.cpp */
    // Point based conversions
    string pointToCellId(Point &point);
    pair<uint64_t, uint64_t> pointToCellCoords(Point &point);
    string pointToKey(Point &point);
    Cell pointToCell(Point &point);
    // String based conversions
    Point keyToPoint(string &key);
    // Value based conversions
    string valuesToCellId(uint64_t x, uint64_t y);
    Cell valuesToCell(uint64_t &x, uint64_t &y);
    // Distance metric between points
    uint64_t euclideanDistance(Point &p1, Point &p2);
};

#endif // DATAGRID_HH
