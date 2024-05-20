#include "GridModel.hh"

pair <uint64_t, uint64_t> Grid::valuesToCellId(uint64_t x, uint64_t y) {
//    std::ostringstream oss;
//    oss << x << ',' << y;
    return make_pair(x, y);
}

Cell Grid::pointToCell(Point &point) {
    uint64_t coordX = point.x / 500;
    uint64_t coordY = point.y / 500;
    pair <uint64_t, uint64_t> id = valuesToCellId(coordX, coordY);

    return Cell(id, coordX, coordY);
}

uint64_t Grid::euclideanDistance(Point &p1, Point &p2) {
    uint64_t dx = std::abs(static_cast<int64_t>(p1.x) - static_cast<int64_t>(p2.x));
    uint64_t dy = std::abs(static_cast<int64_t>(p1.y) - static_cast<int64_t>(p2.y));
    return dx * dx + dy * dy;
}
