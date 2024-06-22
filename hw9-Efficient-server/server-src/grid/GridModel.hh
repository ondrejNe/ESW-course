
#ifndef GRID_MODEL_HH
#define GRID_MODEL_HH

#include <unordered_map>
#include <map>
#include <set>
#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <limits>
#include <cstdint>
#include <shared_mutex>
#include <string>
#include <future>
#include <utility>
#include <mutex>

#include "scheme.pb.h"
#include "robin_map.h"

#include "Logger.hh"

// Global variables -------------------------------------------------------------------------------
#define ONE_TO_ONE  false
#define ONE_TO_ALL  true

extern std::shared_mutex rwLock;

// Class definition -------------------------------------------------------------------------------
using namespace std;

// model
class GridStats {
private:
public:
    uint64_t edges_count;
    pair <uint64_t, uint64_t> highestCoordX;
    pair <uint64_t, uint64_t> highestCoordY;
    pair <uint64_t, uint64_t> lowestCoordX;
    pair <uint64_t, uint64_t> lowestCoordY;

    uint64_t walk_count;
    uint64_t oneToOne_count;
    uint64_t oneToAll_count;
    uint64_t location_count;

    GridStats() {
        edges_count = 0;
        highestCoordX = {numeric_limits<uint64_t>::min(), 0};
        highestCoordY = {0, numeric_limits<uint64_t>::min()};
        lowestCoordX = {numeric_limits<uint64_t>::max(), 0};
        lowestCoordY = {0, numeric_limits<uint64_t>::max()};

        walk_count = 0;
        oneToOne_count = 0;
        oneToAll_count = 0;
        location_count = 0;
    }

    void logGridStats();
};

struct Point {
    uint64_t x;
    uint64_t y;
};

struct Edge {
    uint64_t id;
    uint64_t length;
    uint64_t samples;
};

struct Cell {
    uint64_t id;
    uint64_t coordX;
    uint64_t coordY;
    uint64_t pointX;
    uint64_t pointY;
    vector<Edge> edges;
};

class GridData {
private:
public:
    tsl::robin_map<uint64_t, Cell> cells;

    GridData() {
        cells.reserve(115000);
    }

    uint64_t getPointCellId(Point &point);

    void addEdge(GridStats &gridStats, uint64_t &originCellId, uint64_t &destinationCellId, uint64_t length);

    void addPoint(GridStats &gridStats, Point &point, uint64_t &cellId);

    void resetGrid(GridStats &gridStats);

    void logGridGraph();
};

// processing
uint64_t dijkstra(GridData &gridData, uint64_t &originCellId, uint64_t &destinationCellId, bool oneToAll);

void processWalk(GridData &gridData, GridStats &gridStats, const esw::Walk &walk);

void processReset(GridData &gridData, GridStats &gridStats);

uint64_t processOneToOne(GridData &gridData, GridStats &gridStats, const esw::OneToOne &oneToOne);

uint64_t processOneToAll(GridData &gridData, GridStats &gridStats, const esw::OneToAll &oneToAll);


#endif //GRID_MODEL_HH
