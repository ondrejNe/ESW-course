
#ifndef HW9_EFFICIENT_SERVER_EPOLLINSTANCE_H
#define HW9_EFFICIENT_SERVER_EPOLLINSTANCE_H

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
#include <unordered_map>
#include <map>

#include "EpollEntry.hh"

#include "Logger.hh"

// Global variables -------------------------------------------------------------------------------

// Class definition -------------------------------------------------------------------------------
class EpollInstance
{
private:
    int fd;
    std::map<int, std::unique_ptr<EpollEntry>> entries;

public:
    EpollInstance() {
        this->fd = epoll_create1(EPOLL_CLOEXEC);
        if (this->fd == -1) {
            throw runtime_error(string("epoll_create1: ") + strerror(errno));
        }
        this->entries = std::map<int, std::unique_ptr<EpollEntry>>();
    }

    ~EpollInstance() {
        close(this->fd);
    }

    void registerEpollEntry(std::unique_ptr<EpollEntry> e);

    void unregisterEpollEntry(int fd);

    void waitAndHandleEvents();

    void set_fd(int i) {
        this->fd = i;
    }

    int get_fd() const {
        return this->fd;
    }
};

#endif //HW9_EFFICIENT_SERVER_EPOLLINSTANCE_H
