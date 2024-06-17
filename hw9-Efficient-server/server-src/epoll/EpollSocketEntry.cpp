
#include "EpollModel.hh"

EpollSocketEntry::EpollSocketEntry(uint16_t port, EpollInstance &eSocket, EpollInstance &eConnections, Grid &grid, ThreadPool &resourcePool) :
    eSocket(eSocket),
    eConnections(eConnections),
    grid(grid),
    resourcePool(resourcePool),
    socketLogger("[EPOLL SOCK]", DEBUG),
    connectLogger("[EPOLL CONN]", DEBUG),
    processLogger("[EPOLL PROC]", DEBUG)
{
    int fd;
    struct sockaddr_in addr;

    /** 
     * Creates a new socket
    */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        throw runtime_error("Socket creation failed: " + string(strerror(errno)));
    }
    std::ostringstream oss;
    oss << "[FD: " << fd << "]";
    string fdPrefix = oss.str();
    socketLogger.addPrefix(fdPrefix);
    socketLogger.info("Socket created (%d)", fd);

    /**
     * Setting the socket to non-blocking mode
    */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(fd);
        throw runtime_error("Failed to set socket non-blocking: " + string(strerror(errno)));
    }

    /**
     * By setting the SO_REUSEADDR option, the socket is allowed to reuse the local address it is bound to. 
     * This can be useful when restarting a server application, as it avoids the "Address already in use" 
     * error that can occur when attempting to bind a socket to an address that was recently used by a 
     * previous instance of the application.
    */
    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
        close(fd);
        throw runtime_error("Socket option setting failed: " + string(strerror(errno)));
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    /**
     * Bind the socket to an address
    */
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(fd);
        throw runtime_error("Socket binding failed: " + string(strerror(errno)));
    }
    socketLogger.info("Socket bound to port: %d", port);

    /**
     * Listen for connections on the socket. n connections is queued before refusal of new ones.
    */
    if (listen(fd, SOMAXCONN) == -1) {
        close(fd);
        throw runtime_error("Socket listen failed: " + string(strerror(errno)));
    }
    socketLogger.info("Socket listening for connections");

    // Set the file descriptor and events for the epoll entry
    this->set_fd(fd);
    this->set_events(EPOLLIN | EPOLLET | EPOLLHUP | EPOLLRDHUP | EPOLLONESHOT);
}

bool EpollSocketEntry::handleEvent(uint32_t events)
{
    /**
     * EPOLLERR indicates that an error occurred on the associated file descriptor. 
     * EPOLLHUP indicates that a hang-up occurred on the associated file descriptor.
     * EPOLLIN indicates that the associated file descriptor is ready for reading.
    */
    if ((events & EPOLLERR) || (events & EPOLLHUP) ) { // An incoming connection should only trigger EPOLLIN
        return false; // Something went wrong, remove the socket
    }

    // fcntl can trigger the epoll event other than EPOLLIN, but we don't want to handle it
    if (events & EPOLLIN) {

        int connFd;
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        // Accept the connection and make it non-blocking
        connFd = accept(this->get_fd(), (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (connFd == -1) {
            socketLogger.error("Failed to accept connection: %s", std::string(strerror(errno)));
            return false;
        }
        socketLogger.info("Socket accepted connection (%d)", connFd);

        // Set the connection fd to non-blocking mode
        int flags = fcntl(connFd, F_GETFL, 0);
        if (flags == -1 || fcntl(connFd, F_SETFL, flags | O_NONBLOCK) == -1) {
            close(connFd);
            socketLogger.error("Failed to set connection non-blocking: %s", std::string(strerror(errno)));
            return false;
        }

        // Create a new EpollConnection and register it
        EpollConnectEntry *conn = new EpollConnectEntry(connFd, grid, resourcePool, eConnections, connectLogger, processLogger);
        eConnections.registerEpollEntry(*conn);
        socketLogger.debug("Socket registered connection (%d)", connFd);
    }

    return true; // The listening socket should remain active
}
