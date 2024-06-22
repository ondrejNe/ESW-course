#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "protobuf/scheme.pb.h"

#include "EpollConnectEntry.hh"
#include "EpollSocketEntry.hh"
#include "EpollInstance.hh"

#include "GridModel.hh"
#include "ThreadPool.hh"
#include "Logger.hh"

using namespace std;
using boost::asio::ip::tcp;

// Global variables -------------------------------------------------------------------------------
PrefixedLogger logger = PrefixedLogger("[SERVER APP]", true);

ThreadPool resourcePool(1);
ThreadPool resourcePool1(30);
Grid grid = Grid();

// Main function -----------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    unsigned short int port;
    if (argc != 2) {
        cout << "[ERROR] One argument required <port>" << endl;
        port = 4321;
    } else {
        port = atoi(argv[1]);
    }
    logger.info("Server started on port " + to_string(port));
#ifdef ENABLE_LOGGER_FILE
    ofstream outputFile("log.txt");  // Open the file for writing
    cout.rdbuf(outputFile.rdbuf());  // Redirect cout to the file
#endif

    uint64_t numThreads = thread::hardware_concurrency();
    logger.info("Available threads: " + to_string(numThreads));

    // For logging purposes perform suspensions
    this_thread::sleep_for(chrono::milliseconds(1));

    EpollInstance epollInstance;

    // For logging purposes perform suspensions
    this_thread::sleep_for(chrono::milliseconds(1));

    // Socket events
    thread ep = thread([&epollInstance]() {
        while (true) epollInstance.waitAndHandleEvents();
    });

    // Calculation logic
    std::unique_ptr<EpollSocketEntry> serverSocket = std::make_unique<EpollSocketEntry>(port, epollInstance);
    epollInstance.registerEpollEntry(std::move(serverSocket));

    resourcePool.waitAllThreads();
    return 0;
}
