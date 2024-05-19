#include "GridModel.hh"

string Grid::valuesToCellId(uint64_t x, uint64_t y) {
    return to_string(x) + ',' + to_string(y);
}

Cell Grid::pointToCell(Point& point) {
    uint64_t coordX = point.x / 500;
    uint64_t coordY = point.y / 500;
    std::string id = valuesToCellId(coordX, coordY);

    return Cell(id, coordX, coordY);
}

uint64_t Grid::euclideanDistance(Point &p1, Point &p2) {
    uint64_t dx = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;
    uint64_t dy = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
    uint64_t dSquared = dx * dx + dy * dy;
    return dSquared;
}
