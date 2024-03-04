#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "EpollSocket.hh"
#include "EpollConnection.hh"

/**
 * In this specific case, the EpollInstance &epollInstance passed as a 
 * parameter to the EpollSocket constructor is used to initialize the 
 * epollInstance member variable of the class.
 * Using an initialization list is a good practice because it allows 
 * the member variables to be directly initialized with their final 
 * values, instead of being assigned later in the constructor's body. 
 * This can lead to better performance and helps avoid potential issues
 *  with uninitialized variables.
*/
EpollSocket::EpollSocket(uint16_t port, EpollInstance &epollInstance)
    : epollInstance(epollInstance)
{
    int fd;
    struct sockaddr_in addr;

    /** 
     * Creates a new socket
     * AF_INET: This argument specifies the address family, which is IPv4 in this case.
     * SOCK_STREAM: This argument specifies the socket type, which is a reliable, 
     * connection-oriented, byte-stream socket (TCP) in this case.
     * 0: This argument specifies the protocol to be used. 0 means that the system will 
     * choose the appropriate protocol based on the socket type. For SOCK_STREAM, the default protocol is TCP.
    */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        throw std::runtime_error("Socket creation failed: " + std::string(std::strerror(errno)));
    }

    /**
     * Setting the socket to non-blocking mode
     * F_GETFL command, which retrieves the file descriptor flags for the socket referred to by fd
     * The code checks if the retrieved flags are -1, which would indicate an error in fetching the 
     * flags. If an error occurs, it proceeds to close the socket and throw a runtime error.
     * fcntl(fd, F_SETFL, flags | O_NONBLOCK): The fcntl() function is called again, this time 
     * with the F_SETFL command to set the file descriptor flags. The new flags are set by OR-ing 
     * the current flags with O_NONBLOCK, which is the flag for non-blocking mode.
     * 
    */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(fd);
        throw std::runtime_error("Failed to set socket non-blocking: " + std::string(std::strerror(errno)));
    }

    /**
     * Set a socket option for the socket referred to by the file descriptor fd. The specific option 
     * being set is SO_REUSEADDR, which allows the socket to reuse the local address (usually the IP 
     * and port combination) it is bound to.
     * 
     * By setting the SO_REUSEADDR option, the socket is allowed to reuse the local address it is bound to. 
     * This can be useful when restarting a server application, as it avoids the "Address already in use" 
     * error that can occur when attempting to bind a socket to an address that was recently used by a 
     * previous instance of the application.
    */
    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
        close(fd);
        throw std::runtime_error("Socket option setting failed: " + std::string(std::strerror(errno)));
    }

    addr.sin_family = AF_INET;
    /* 
        The htons function takes a 16-bit number in host byte order and returns a 16-bit number in network 
        byte order used in TCP/IP networks (the AF_INET or AF_INET6 address family).
    */
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    /**
     * Bind the socket to an address
    */
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(fd);
        throw std::runtime_error("Socket binding failed: " + std::string(std::strerror(errno)));
    }

    /**
     * Listen for connections on the socket. n connections is queued before refusal of new ones.
    */
    if (listen(fd, SOMAXCONN) == -1) {
        close(fd);
        throw std::runtime_error("Socket listen failed: " + std::string(std::strerror(errno)));
    }

    // Set the file descriptor and events for the epoll entry
    this->set_fd(fd);
    this->set_events(EPOLLIN);
}

EpollSocket::~EpollSocket()
{
    // Close the listening socket
    close(this->get_fd());
}

bool
EpollSocket::handleEvent(uint32_t events)
{
    /**
     * EPOLLERR indicates that an error occurred on the associated file descriptor. 
     * EPOLLHUP indicates that a hang-up occurred on the associated file descriptor, 
     * which usually means the connection was closed by the remote peer or an error occurred.
     * EPOLLIN indicates that the associated file descriptor is ready for reading.
    */
    // An incoming connection should only trigger EPOLLIN
    if ((events & EPOLLERR) || (events & EPOLLHUP) || !(events & EPOLLIN)) {
        return false; // Something went wrong, remove the socket
    } else {
        int connFd;
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        // Accept the connection and make it non-blocking
        connFd = accept(this->get_fd(), (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (connFd == -1) {
            std::cerr << "Failed to accept connection" << std::endl;
        } else {
            // Set the fd to non-blocking mode
            int flags = fcntl(connFd, F_GETFL, 0);
            if (flags == -1 || fcntl(connFd, F_SETFL, flags | O_NONBLOCK) == -1) {
                close(connFd);
                throw std::runtime_error("Failed to set connection non-blocking: " + std::string(std::strerror(errno)));
            }

            // Create a new EpollConnection and register it
            EpollConnection *conn = new EpollConnection(connFd);
            epollInstance.registerEpollEntry(*conn);
            std::cout << "Accepted connection with file descriptor: " << connFd << std::endl;
        }

        return true; // The listening socket should remain active
    }
}
