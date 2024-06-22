
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

extern ThreadPool writePool;
extern ThreadPool readPool;
extern Grid grid;

// Class definition -------------------------------------------------------------------------------
class EpollConnectEntry : public EpollEntry
{
private:
    // Connection state
    int             inProgressMessageSize;
    int             inProgressMessageOffset;
    char            messageBuffer[50000];
    bool            messageInProgress;

    // Message content
    esw::Request    request;
    esw::Response   response;

    // Message content state
    std::mutex      inProgressWalksMutex;
    int             inProgressWalks;
    bool            waitingForWalksProcessed;

    void readEvent();

    int readMessageSize();

    void processWalk(esw::Request request, esw::Response response, int fd);

    void processOneToOne(esw::Request request, esw::Response response, Grid grid, int fd);

    void processOneToAll(esw::Request request, esw::Response response, Grid grid, int fd);

    void processReset(esw::Response response, int fd);

    void processError(esw::Response response, int fd);

    void writeResponse(esw::Response &response, int fd);

public:
    // A proper constructor for an accepted connection
    EpollConnectEntry(int fd) :
            inProgressMessageSize(0),
            inProgressMessageOffset(0),
            messageInProgress(false),
            request(esw::Request()),
            response(esw::Response()),
            inProgressWalks(0),
            waitingForWalksProcessed(false) {

        // Assign the file descriptor of the accepted connection
        this->set_fd(fd);
        this->set_events(EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP | EPOLLONESHOT);
        // Add logging
#ifdef CONNECT_LOGGER
        connectLogger.info("Connection epoll entry created FD%d", fd);
#endif
    }

    ~EpollConnectEntry() {
#ifdef CONNECT_LOGGER
        connectLogger.info("Connection epoll entry closed FD%d", this->get_fd());
#endif
    }

    // Handle incoming data or errors for the connection
    bool handleEvent(uint32_t events);

    // Cleanup on disconnect
    void Cleanup();
};

#endif //HW9_EFFICIENT_SERVER_EPOLLCONNECTENTRY_H
