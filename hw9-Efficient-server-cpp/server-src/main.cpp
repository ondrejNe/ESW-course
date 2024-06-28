#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "protobuf/scheme.pb.h"

#include "EpollConnectEntry.hh"
#include "EpollSocketEntry.hh"
#include "EpollInstance.hh"

#include "Logger.hh"
#include "GridModel.hh"
#include "ThreadPool.hh"

using namespace std;

// Global variables -------------------------------------------------------------------------------
PrefixedLogger logger = PrefixedLogger("[SERVER APP]", true);

ThreadPool resourcePool(1);
ThreadPool resourcePool1(20);
GridData gridData = GridData();
GridStats gridStats = GridStats();

// Main function -----------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    unsigned short int port;
    if (argc != 2) {
        cout << "[ERROR] One argument required <port>" << endl;
        port = 4444;
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
    uint64_t numCores = sysconf(_SC_NPROCESSORS_ONLN);
    logger.info("Available cores: " + to_string(numCores));

    // Epoll
    EpollInstance epollInstance;

    resourcePool1.run([&epollInstance]() {
        while (true) epollInstance.waitAndHandleEvents();
    }, -1);

    std::unique_ptr<EpollSocketEntry> serverSocket = std::make_unique<EpollSocketEntry>(port, epollInstance);
    epollInstance.registerEpollEntry(std::move(serverSocket));

    resourcePool.waitAllThreads();
    resourcePool1.waitAllThreads();
    return 0;
}
