
#ifndef EPOLL_MODEL_HH
#define EPOLL_MODEL_HH

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <cstring>
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "GridModel.hh"
#include "Logger.hh"
#include "ThreadPool.hh"

#define EPOLL_MAX_EVENTS 1024

using namespace std;
using namespace fmt;

/** -------------------------------------------------------------------------------------------- */
class EpollEntry
{
private:
    int fd;
    uint32_t events;
public:
    virtual bool handleEvent(uint32_t events) = 0;

    void set_fd(int i) {
        this->fd = i;
    }

    int get_fd() const {
        return this->fd;
    }

    void set_events(uint32_t i) {
        this->events = i;
    }

    uint32_t get_events() const {
        return this->events;
    }

    virtual ~EpollEntry() = default;
};

/** -------------------------------------------------------------------------------------------- */
class EpollInstance
{
private:
    int fd;
    PrefixedLogger epollLogger;

public:
    EpollInstance() : epollLogger("[EPOLL INSTANCE]", DEBUG) {
        this->fd = epoll_create1(0);
        if (this->fd == -1) {
            throw runtime_error(string("epoll_create1: ") + strerror(errno));
        }
        string fdPrefix = fmt::format("[fd: {}]", this->fd);
        epollLogger.addPrefix(fdPrefix);
    }

    ~EpollInstance() {
        close(this->fd);
    }

    void registerEpollEntry(EpollEntry &e);

    void unregisterEpollEntry(EpollEntry &e);

    void waitAndHandleEvents();

    void set_fd(int i) {
        this->fd = i;
    }

    int get_fd() const {
        return this->fd;
    }
};

/** -------------------------------------------------------------------------------------------- */
class EpollSocketEntry : public EpollEntry
{
public:
    // Constructor creates the listening socket
    EpollSocketEntry(uint16_t port, EpollInstance &eSocket, EpollInstance &eConnections, Grid &grid, ThreadPool &resourcePool);

    // Accept connections and create epoll connection entries
    bool handleEvent(uint32_t events);

private:
    EpollInstance &eSocket; // Reference to the epoll instance
    EpollInstance &eConnections; // Reference to the epoll instance
    Grid &grid;
    ThreadPool &resourcePool;
    PrefixedLogger socketLogger;
};

/** -------------------------------------------------------------------------------------------- */
class EpollConnectEntry : public EpollEntry {
public:
    // A proper constructor for an accepted connection
    EpollConnectEntry(int fd, Grid &grid, ThreadPool &resourcePool);

    // Cleanup on disconnect
    void Cleanup();

    // Handle incoming data or errors for the connection
    bool handleEvent(uint32_t events);

private:
    // Reading functions
    bool readEvent();

    int readMessageSize();

    // Writing functions
    void writeResponse(esw::Response &response);

    // Shared resource pointer
    Grid &grid;
    ThreadPool &resourcePool;
    PrefixedLogger connectLogger;
    // Single message variables
    bool messageInProgress;
    char *messageBuffer;
    int inProgressMessageSize;
    int inProgressMessageRead;
};

#endif //EPOLL_MODEL_HH