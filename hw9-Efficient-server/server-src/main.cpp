#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "protobuf/scheme.pb.h"

#include "EpollModel.hh"
#include "GridModel.hh"
#include "ThreadPool.hh"
#include "Logger.hh"

using namespace std;
using boost::asio::ip::tcp;

// Global variables -------------------------------------------------------------------------------
PrefixedLogger logger = PrefixedLogger("[SERVER APP]", INFO);

// Main function -----------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    unsigned short int port;
    if (argc != 2) {
        cout << "[ERROR] One argument required <port>" << endl;
        port = 4321;
    } else {
        port = atoi(argv[1]);
    }
    cout << "Server started on port " << port << endl;
#ifdef ENABLE_LOGGER_FILE
    ofstream outputFile("log.txt");  // Open the file for writing
    cout.rdbuf(outputFile.rdbuf());  // Redirect cout to the file
#endif

    uint64_t numThreads = thread::hardware_concurrency();
    logger.info("Available threads: " + to_string(numThreads));
    logger.info("Pool one  threads: %d", RESOURCE_POOL_ONE_SIZE);
    logger.info("Pool two  threads: %d", RESOURCE_POOL_TWO_SIZE);
    logger.info("Listening on port: " + to_string(port));

    // For logging purposes perform suspensions
    this_thread::sleep_for(chrono::milliseconds(1));

    /* Prepare server resources */
    ThreadPool resourcePool(RESOURCE_POOL_ONE_SIZE);
    ThreadPool gridPool(RESOURCE_POOL_TWO_SIZE);

    EpollInstance epollConnectInstance;
    EpollInstance epollSocketInstance;

    // For logging purposes perform suspensions
    this_thread::sleep_for(chrono::milliseconds(1));

    // Connection events
    resourcePool.run([&epollConnectInstance]() {
        while (true) epollConnectInstance.waitAndHandleEvents();
    });
    // Socket events
    resourcePool.run([&epollSocketInstance]() {
        while (true) epollSocketInstance.waitAndHandleEvents();
    });

    /* Start the server */
    Grid grid = Grid(gridPool);
    EpollSocketEntry serverSocket(port, epollSocketInstance, epollConnectInstance, grid, resourcePool);
    epollSocketInstance.registerEpollEntry(serverSocket);

    /* Wait */
    resourcePool.waitAllThreads();
    return 0;
}
