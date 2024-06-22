
#ifndef HW9_EFFICIENT_SERVER_EPOLLSOCKETENTRY_H
#define HW9_EFFICIENT_SERVER_EPOLLSOCKETENTRY_H

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
#include "EpollConnectEntry.hh"

#include "Logger.hh"
#include "GridModel.hh"
#include "ThreadPool.hh"

// Global variables -------------------------------------------------------------------------------
extern PrefixedLogger socketLogger;

// Class definition -------------------------------------------------------------------------------
class EpollSocketEntry : public EpollEntry
{
private:
    EpollInstance   &epollInstance;
public:
    // Constructor creates the listening socket
    EpollSocketEntry(uint16_t port, EpollInstance &epollInstance);

    // Accept connections and create epoll connection entries
    bool handleEvent(uint32_t events) override;
};

#endif //HW9_EFFICIENT_SERVER_EPOLLSOCKETENTRY_H
