
#ifndef HW9_EFFICIENT_SERVER_EPOLLCONNECTENTRY_H
#define HW9_EFFICIENT_SERVER_EPOLLCONNECTENTRY_H

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>

#include "EpollEntry.hh"
#include "EpollInstance.hh"
#include "EpollSocketEntry.hh"

#include "Logger.hh"
#include "GridModel.hh"
#include "ThreadPool.hh"

// Global variables -------------------------------------------------------------------------------
extern PrefixedLogger connectLogger;
extern PrefixedLogger processLogger;

extern ThreadPool resourcePool;
extern Grid grid;

// Class definition -------------------------------------------------------------------------------
class EpollConnectEntry : public EpollEntry
{
private:
    int             inProgressMessageSize;
    char            messageBuffer[50000];
    bool            processingInProgress;

    void readEvent();

    int readMessageSize();

    void processMessage(esw::Request request, esw::Response response, int fd);

    void writeResponse(esw::Response &response, int fd);

public:
    // A proper constructor for an accepted connection
    EpollConnectEntry(int fd) :
            inProgressMessageSize(0),
            processingInProgress(false) {

        // Assign the file descriptor of the accepted connection
        this->set_fd(fd);
        this->set_events(EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLONESHOT);
        // Add logging
        connectLogger.info("Connection epoll entry created FD%d", fd);
    }

    ~EpollConnectEntry() {
        connectLogger.info("Connection epoll entry closed FD%d", this->get_fd());
    }

    // Handle incoming data or errors for the connection
    bool handleEvent(uint32_t events);

    // Cleanup on disconnect
    void Cleanup();
};

#endif //HW9_EFFICIENT_SERVER_EPOLLCONNECTENTRY_H
