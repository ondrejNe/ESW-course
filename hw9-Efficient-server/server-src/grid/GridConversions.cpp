#include "GridModel.hh"

string Grid::pointToCellId(Point &point) {
    return to_string(point.x / 500) + ',' + to_string(point.y / 500);
}

pair<uint64_t, uint64_t> Grid::pointToCellCoords(Point &point) {
    return make_pair(point.x / 500, point.y / 500);
}

string Grid::valuesToCellId(uint64_t x, uint64_t y) {
    return to_string(x) + ',' + to_string(y);
}

string Grid::pointToKey(Point &point) {
    return to_string(point.x) + ',' + to_string(point.y);
}

Point Grid::keyToPoint(string& key) {
    vector<string> coordinates;
    boost::split(coordinates, key, boost::is_any_of(","));
    return {static_cast<uint64_t>(std::stoi(coordinates[0])),
            static_cast<uint64_t>(std::stoi(coordinates[1]))};
}

Cell Grid::pointToCell(Point &point) {
    Cell newCell;
    newCell.coordX = point.x / 500;
    newCell.coordY = point.y / 500;
    newCell.id = valuesToCellId(newCell.coordX, newCell.coordY);
    return newCell;
}

Cell Grid::valuesToCell(uint64_t &x, uint64_t &y) {
    Cell newCell;
    newCell.coordX = x;
    newCell.coordY = y;
    newCell.id = valuesToCellId(newCell.coordX, newCell.coordY);
    return newCell;
}

uint64_t Grid::euclideanDistance(Point &p1, Point &p2) {
    uint64_t dx = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;
    uint64_t dy = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
    uint64_t dSquared = dx * dx + dy * dy;
    return dSquared;
}
