#include "DataGrid.hh"

string DataGrid::pointToCellId(Point &point) {
    return to_string(point.x / 500) + ',' + to_string(point.y / 500);
}

pair<uint64_t, uint64_t> DataGrid::pointToCellCoords(Point &point) {
    return make_pair(point.x / 500, point.y / 500);
}

string DataGrid::valuesToCellId(uint64_t x, uint64_t y) {
    return to_string(x) + ',' + to_string(y);
}

string DataGrid::pointToKey(Point &point) {
    return to_string(point.x) + ',' + to_string(point.y);
}

Point DataGrid::keyToPoint(string& key) {
    vector<string> coordinates;
    boost::split(coordinates, key, boost::is_any_of(","));
    return {static_cast<uint64_t>(std::stoi(coordinates[0])),
            static_cast<uint64_t>(std::stoi(coordinates[1]))};
}

Cell DataGrid::pointToCell(Point &point) {
    Cell newCell;
    newCell.coordX = point.x / 500;
    newCell.coordY = point.y / 500;
    newCell.id = valuesToCellId(newCell.coordX, newCell.coordY);
    return newCell;
}

Cell DataGrid::valuesToCell(uint64_t &x, uint64_t &y) {
    Cell newCell;
    newCell.coordX = x;
    newCell.coordY = y;
    newCell.id = valuesToCellId(newCell.coordX, newCell.coordY);
    return newCell;
}

uint64_t DataGrid::euclideanDistance(Point &p1, Point &p2) {
    // cout << "[EUCLIDEAN] Calculating distance between: " << p1.x << "," << p1.y << " and " << p2.x << "," << p2.y << endl;
    uint64_t dx = p1.x > p2.x ? p1.x - p2.x : p2.x - p1.x;
    // cout << "[EUCLIDEAN] dx: " << dx << endl;
    uint64_t dy = p1.y > p2.y ? p1.y - p2.y : p2.y - p1.y;
    // cout << "[EUCLIDEAN] dy: " << dy << endl;
    uint64_t dSquared = dx * dx + dy * dy;
    // cout << "[EUCLIDEAN] Distance: " << dSquared << endl;
    return dSquared;
}
