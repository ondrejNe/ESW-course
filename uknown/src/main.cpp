#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/epoll.h>
// Protobuf
#include "protobuf/scheme.pb.h"
// DataGrid
#include "data/DataGrid.hh"
// Epoll
#include "epoll/EpollConnection.hh"
#include "epoll/EpollSocket.hh"
#include "epoll/EpollInstance.hh"
#include "epoll/EpollEntry.hh"
// ThreadPool
#include "threadpool/ThreadPool.hh"

using namespace std;
using boost::asio::ip::tcp;

// Global variables -----------------------------------------------------
DataGrid dataGrid = DataGrid();

// Main function --------------------------------------------------------
int main(int argc, char *argv[]) {
    /* Parameter check */
    if (argc != 2) {
        cout << "[SERVER] Error: one arguments required - ./server <port>" << endl;
        return 1;
    }

    /* Server parameters */
    unsigned short int port = atoi(argv[1]);
    uint64_t numThreads = thread::hardware_concurrency();
    cout << "[SERVER] Number of threads: " << numThreads << endl;
    cout << "[SERVER] Listening on port: " << port << endl;

    /* Output logging */
    ofstream outputFile("log.txt");  // Open the file for writing
    // Redirect cout to the file stream
    streambuf* originalBuffer = cout.rdbuf();  // Save the original buffer
    cout.rdbuf(outputFile.rdbuf());  // Redirect cout to the file

    /* Prepare server resources */
    /** Write to data structure */
    // ThreadPool writerPool(1);

    /** Read in datastructure */
    // ThreadPool readerPool(1);

    /** Handle connection communication */
    EpollInstance connectionsEpoll;
    EpollInstance socketEpoll;

    ThreadPool resourcePool(2);
    /** Handle connection communication */
    resourcePool.enqueue([&connectionsEpoll]() {
        while (true) connectionsEpoll.waitAndHandleEvents();
    });
    /** Handle new connections */
    resourcePool.enqueue([&socketEpoll]() {
        while (true) socketEpoll.waitAndHandleEvents();
    });

    /* Start the server */
    EpollSocket serverSocket(port, socketEpoll, connectionsEpoll, dataGrid, resourcePool);
    socketEpoll.registerEpollEntry(serverSocket);
    
    /* Wait */
    // writerPool.waitAllThreads();
    // readerPool.waitAllThreads();
    resourcePool.waitAllThreads();

    /* Close the file */
    cout.rdbuf(originalBuffer);  // Restore the original buffer
    return 0;
}
