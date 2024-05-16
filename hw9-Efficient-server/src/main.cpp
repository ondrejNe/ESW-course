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

// Global variables -----------------------------------------------------
Grid grid = Grid();
PrefixedLogger logger = PrefixedLogger("[App]", INFO);

// Main function --------------------------------------------------------
int main(int argc, char *argv[]) {
    /* Parameter check */
    if (argc != 2) {
        logger.error("Error: one arguments required - ./server <port>");
        return 1;
    }

    /* Server parameters */
    unsigned short int port = atoi(argv[1]);
    uint64_t numThreads = thread::hardware_concurrency();
    logger.info("Number of threads: " + to_string(numThreads));
    logger.info("Listening on port: " + to_string(port));

    /* Output logging */
    ofstream outputFile("log.txt");  // Open the file for writing
    // Redirect cout to the file stream
    streambuf *originalBuffer = cout.rdbuf();  // Save the original buffer
    cout.rdbuf(outputFile.rdbuf());  // Redirect cout to the file

    /* Prepare server resources */
    ThreadPool writerPool(1);
    ThreadPool readerPool(1);

    EpollInstance connectionsEpoll;
    EpollInstance socketEpoll;

    ThreadPool resourcePool(2);

    resourcePool.enqueue([&connectionsEpoll]() {
        while (true) connectionsEpoll.waitAndHandleEvents();
    });
    resourcePool.enqueue([&socketEpoll]() {
        while (true) socketEpoll.waitAndHandleEvents();
    });

    /* Start the server */
    EpollSocket serverSocket(port, socketEpoll, connectionsEpoll, grid, resourcePool);
    socketEpoll.registerEpollEntry(serverSocket);

    /* Wait */
    writerPool.waitAllThreads();
    readerPool.waitAllThreads();
    resourcePool.waitAllThreads();

    /* Close the file */
    cout.rdbuf(originalBuffer);  // Restore the original buffer
    return 0;
}
