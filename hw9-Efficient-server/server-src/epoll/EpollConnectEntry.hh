
#ifndef HW9_EFFICIENT_SERVER_EPOLLCONNECTENTRY_H
#define HW9_EFFICIENT_SERVER_EPOLLCONNECTENTRY_H

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
#include <memory>
#include <functional>

#include "EpollEntry.hh"

#include "Logger.hh"
#include "GridModel.hh"
#include "ThreadPool.hh"

// Global variables -------------------------------------------------------------------------------
extern PrefixedLogger connectLogger;

extern ThreadPool resourcePool1;
extern ThreadPool resourcePool2;

// Class definition -------------------------------------------------------------------------------
class EpollConnectEntry : public EpollEntry
{
private:
    int             inProgressMessageSize;
    int             inProgressMessageOffset;
    char            messageBuffer[50000];
    bool            messageInProgress;
    bool            processingInProgress;

    void readEvent(GridData &gridData, GridStats &gridStats);

    int readMessageSize();

    void processMessage(esw::Request request, esw::Response response, GridData &gridData, GridStats &gridStats, int fd);

    void writeResponse(esw::Response &response, int fd);

public:
    // A proper constructor for an accepted connection
    EpollConnectEntry(int fd) :
            inProgressMessageSize(0),
            inProgressMessageOffset(0),
            messageInProgress(false),
            processingInProgress(false) {

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
    bool handleEvent(uint32_t events, GridData &gridData, GridStats &gridStats);

    // Cleanup on disconnect
    void Cleanup();
};

#endif //HW9_EFFICIENT_SERVER_EPOLLCONNECTENTRY_H
