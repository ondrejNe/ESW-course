#include "GridModel.hh"

string Grid::valuesToCellId(uint64_t x, uint64_t y) {
    return to_string(x) + ',' + to_string(y);
}

Cell Grid::pointToCell(Point &point) {
    Cell newCell;
    newCell.coordX = point.x / 500;
    newCell.coordY = point.y / 500;
    newCell.id = valuesToCellId(newCell.coordX, newCell.coordY);
    return newCell;
}

uint64_t Grid::euclideanDistance(Point &p1, Point &p2) {
    uint64_t dx = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;
    uint64_t dy = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
    uint64_t dSquared = dx * dx + dy * dy;
    return dSquared;
}
