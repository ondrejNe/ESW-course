#include "DataGrid.hh"

// #define DEBUG_CONTENT

void DataGrid::logContent() {
    #ifdef DEBUG_CONTENT
    cout << "[DATA GRID] DataGrid content:" << endl;
    for (const auto& cell : cells) {
        cout << "[DATA GRID]  Cell: " << cell.first << endl;
        cout << "[DATA GRID]    Points: " << endl;
        for (const auto& point : cell.second.points) {
            cout << "[DATA GRID]      x: " << point.x << " y: " << point.y << endl;
        }
        cout << "[DATA GRID]    Edges: " << endl;
        for (const auto& edge : cell.second.edges) {
            cout << "[DATA GRID]      To: " << edge.first << " for: " << edge.second << endl;
        }
    }
    #endif
}
