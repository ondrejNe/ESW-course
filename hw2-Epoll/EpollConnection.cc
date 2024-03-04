#include <iostream>
#include <cstring> 
#include <cerrno>  
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "EpollConnection.hh"

EpollConnection::EpollConnection(int fd)
{
    // Assign the file descriptor of the accepted connection
    this->set_fd(fd);
    this->set_events(EPOLLIN);
}

EpollConnection::~EpollConnection()
{
    // Close the file descriptor when the connection is done
    close(this->get_fd());
}

bool
EpollConnection::handleEvent(uint32_t events)
{
    // Checking for errors or the connection being closed
    if ((events & EPOLLERR) || (events & EPOLLHUP) || !(events & EPOLLIN)) {
        return false; // This connection should be removed
    } else {
        readData(); // Read the data and print it
        return true; // This connection should remain active
    }
}

void 
EpollConnection::readData() {
    char buf[1024];
    ssize_t bytesRead;

    while ((bytesRead = recv(this->get_fd(), buf, sizeof(buf) - 1, 0)) > 0) {
        buf[bytesRead] = '\0';
        receivedData_ += buf;

        std::size_t pos;
        while ((pos = receivedData_.find('\n')) != std::string::npos) {
            int length = pos;

            std::string lengthMessage = std::to_string(length) + "\n";
            ssize_t bytesSent = send(this->get_fd(), lengthMessage.c_str(), lengthMessage.size(), 0);
            if (bytesSent < 0) {
                throw std::runtime_error("Failed to send length message: " + std::string(std::strerror(errno)));
            }

            receivedData_.erase(0, pos + 1);
        }
    }

    if (bytesRead == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
        throw std::runtime_error("Connection closed or error occurred");
    }
}
