
#ifndef HW9_EFFICIENT_SERVER_EPOLLENTRY_H
#define HW9_EFFICIENT_SERVER_EPOLLENTRY_H

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

#define EPOLL_MAX_EVENTS 1024

// Class definition -------------------------------------------------------------------------------
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

#endif //HW9_EFFICIENT_SERVER_EPOLLENTRY_H
