#include <iostream>
#include <fstream>

#include "protobuf/scheme.pb.h"

#include "EpollModel.hh"

#include "GridModel.hh"

#include "ThreadPool.hh"

#include "Locker.hh"
#include "Logger.hh"

using namespace std;
using boost::asio::ip::tcp;

// Global variables -------------------------------------------------------------------------------
Grid grid = Grid();
PrefixedLogger logger = PrefixedLogger("[App]", INFO);

// Main function -----------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    if (argc != 2) {
        logger.error("One argument required <port>");
        return 1;
    }

    /* Server parameters */
    unsigned short int port = atoi(argv[1]);
    uint64_t numThreads = thread::hardware_concurrency();
    logger.info("Number of threads: " + to_string(numThreads));
    logger.info("Listening on port: " + to_string(port));

    /* Output logging */
    ofstream outputFile("log.txt");  // Open the file for writing
    streambuf *originalBuffer = cout.rdbuf();  // Save the original buffer
    cout.rdbuf(outputFile.rdbuf());  // Redirect cout to the file

    /* Prepare server resources */
    ThreadPool resourcePool(RESOURCE_POOL_SIZE);

    EpollInstance epollConnectInstance;
    EpollInstance epollSocketInstance;

    // Connection events
    resourcePool.enqueue([&epollConnectInstance]() {
        while (true) epollConnectInstance.waitAndHandleEvents();
    });
    // Socket events
    resourcePool.enqueue([&epollSocketInstance]() {
        while (true) epollSocketInstance.waitAndHandleEvents();
    });

    /* Start the server */
    EpollSocketEntry serverSocket(port, epollSocketInstance, epollConnectInstance, grid, resourcePool);
    epollSocketInstance.registerEpollEntry(serverSocket);

    /* Wait */
    resourcePool.waitAllThreads();

    /* Close the file */
    cout.rdbuf(originalBuffer);  // Restore the original buffer
    return 0;
}
