
#ifndef EPOLL_MODEL_HH
#define EPOLL_MODEL_HH

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

#include "GridModel.hh"
#include "Logger.hh"
#include "ThreadPool.hh"

#define ACTIVE_LOGGER_EPOLL true
#define EPOLL_MAX_EVENTS 1024

using namespace std;

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

    int is_fd_valid() {
        return fcntl(this->fd, F_GETFD) != -1 || errno != EBADF;
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
    EpollInstance() :
        epollLogger("[EPOLL INST]", ACTIVE_LOGGER_EPOLL) {
            this->fd = epoll_create1(EPOLL_CLOEXEC);
            if (this->fd == -1) {
                throw runtime_error(string("epoll_create1: ") + strerror(errno));
            }
            std::ostringstream oss;
            oss << "[fd: " << fd << "]";
            string fdPrefix = oss.str();
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
private:
    EpollInstance   &eSocket;
    EpollInstance   &eConnections;
    Grid            &grid;
    ThreadPool      &resourcePool;
    PrefixedLogger  socketLogger;
    PrefixedLogger  connectLogger;
    PrefixedLogger  processLogger;
public:
    // Constructor creates the listening socket
    EpollSocketEntry(uint16_t port, EpollInstance &eSocket, EpollInstance &eConnections, Grid &grid, ThreadPool &resourcePool);

    // Accept connections and create epoll connection entries
    bool handleEvent(uint32_t events);
};

/** -------------------------------------------------------------------------------------------- */
class EpollConnectEntry : public EpollEntry
{
private:
    // Shared resource pointer
    Grid            &grid;
    ThreadPool      &resourcePool;
    EpollInstance   &eConnections;
    PrefixedLogger  &connectLogger;
    PrefixedLogger  &processLogger;
    // Single message variables
    int             inProgressMessageSize;
    int             inProgressMessageRead;
    char            *messageBuffer;
    bool            messageInProgress;
    bool            processingInProgress;

    void readEvent();

    int readMessageSize();

    void processMessage(esw::Request request, esw::Response response);

    void writeResponse(esw::Response &response);

public:
    // A proper constructor for an accepted connection
    EpollConnectEntry(int fd, Grid &grid, ThreadPool &resourcePool, EpollInstance &eConnections, PrefixedLogger &connectLogger, PrefixedLogger &processLogger) :
        grid(grid),
        resourcePool(resourcePool),
        eConnections(eConnections),
        connectLogger(connectLogger),
        processLogger(processLogger),
        inProgressMessageSize(0),
        inProgressMessageRead(0),
        messageBuffer(nullptr),
        messageInProgress(false),
        processingInProgress(false) {

        // Assign the file descriptor of the accepted connection
        this->set_fd(fd);
        this->set_events(EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP | EPOLLONESHOT);
        // Add logging
//        ostringstream oss;
//        oss << "[FD: " << fd << "]";
//        string fdPrefix = oss.str();
//        connectLogger.addPrefix(fdPrefix);
        connectLogger.info("Connection created (%d)", fd);
    }

    ~EpollConnectEntry() {
        if (messageBuffer != nullptr) delete[] messageBuffer;
        connectLogger.info("Connection closed (%d)", this->get_fd());
        // Remove logging
        ostringstream oss;
        oss << "[FD: " << this->get_fd() << "]";
        string fdPrefix = oss.str();
        connectLogger.removePrefix(fdPrefix);
    }

    // Handle incoming data or errors for the connection
    bool handleEvent(uint32_t events);

    // Cleanup on disconnect
    void Cleanup();
};

#endif //EPOLL_MODEL_HH
