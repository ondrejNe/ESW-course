
#ifndef GRID_MODEL_HH
#define GRID_MODEL_HH

#include <unordered_map>
#include <map>
#include <set>
#include <cmath>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <queue>
#include <unordered_set>
#include <limits>
#include <cstdint>
#include <shared_mutex>
#include <string>
#include <future>
#include <utility>

#include "scheme.pb.h"
#include "robin_map.h"

#include "Logger.hh"
#include "ThreadPool.hh"
#include "ReentrantSharedLocker.hh"

// Global variables -------------------------------------------------------------------------------
extern PrefixedLogger gridLogger;
extern PrefixedLogger protoLogger;
extern PrefixedLogger searchLogger;

extern ThreadPool writePool;
extern ThreadPool readPool;

extern std::mutex gridMutex;

#define ONE_TO_ONE  false
#define ONE_TO_ALL  true

// Class definition -------------------------------------------------------------------------------
using namespace std;

struct Point {
    uint64_t x;
    uint64_t y;
};

struct Cell {
    uint64_t    id;
    uint64_t    coordX;
    uint64_t    coordY;
    uint64_t    pointX;
    uint64_t    pointY;
    tsl::robin_map<uint64_t, uint64_t> edges;
};

class Grid {
private:
    // Graph structure
    tsl::robin_map <uint64_t, Cell>                              cells;

    // Grid statistics
    uint64_t        edges_count;
    uint64_t        edges_space;
    uint64_t        location_count;
    uint64_t        walk_count;
    uint64_t        oneToOne_count;
    uint64_t        oneToAll_count;

public:
    Grid() {
        cells.reserve(115000);

        edges_count = 0;
        edges_space = 0;
        location_count = 0;
        walk_count = 0;
        oneToOne_count = 0;
        oneToAll_count = 0;
    }

    // Copy constructor
    Grid(const Grid& other) {
        // Perform member-wise copy of data members
        this->cells = other.cells;

        this->edges_count = other.edges_count;
        this->edges_space = other.edges_space;
        this->location_count = other.location_count;
        this->walk_count = other.walk_count;
        this->oneToOne_count = other.oneToOne_count;
        this->oneToAll_count = other.oneToAll_count;
    }

    // Assignment operator
    Grid& operator=(const Grid& other) {
        if (this != &other) { // avoid self-assignment
            std::lock_guard<std::mutex> lock(gridMutex);
            this->cells = other.cells;

            this->edges_count = other.edges_count;
            this->edges_space = other.edges_space;
            this->location_count = other.location_count;
            this->walk_count = other.walk_count;
            this->oneToOne_count = other.oneToOne_count;
            this->oneToAll_count = other.oneToAll_count;
        }
        return *this;
    }

    void addEdge(uint64_t &originCellId, uint64_t &destinationCellId, uint64_t length);

    void addPoint(Point &point, uint64_t &cellId);

    uint64_t getPointCellId(Point &point);

    void resetGrid();

    uint64_t dijkstra(uint64_t &originCellId, uint64_t &destinationCellId, bool oneToAll);


    void processWalk(const esw::Walk &walk);

    void processReset();

    uint64_t processOneToOne(const esw::OneToOne &oneToOne);

    uint64_t processOneToAll(const esw::OneToAll &oneToAll);

    void logGridGraph();

    void logGridStats();
};

extern Grid grid;

#endif //GRID_MODEL_HH
