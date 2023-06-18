#ifndef EPOLL_SOCKET_H
#define EPOLL_SOCKET_H

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "EpollEntry.hh"
#include "EpollConnection.hh"
#include "EpollInstance.hh"

#include "threadpool/ThreadPool.hh"

using namespace std;

class EpollSocket : public EpollEntry
{
public:
    // Constructor creates the listening socket
    EpollSocket(uint16_t port, EpollInstance &eSocket, EpollInstance &eConnections, DataGrid &dataGrid, ThreadPool &resourcePool);
    
    // Accept connections and create epoll connection entries
    bool handleEvent(uint32_t events);

private:
    EpollInstance &eSocket; // Reference to the epoll instance
    EpollInstance &eConnections; // Reference to the epoll instance
    DataGrid &dataGrid;
    ThreadPool &resourcePool;
};

#endif // EPOLL_SOCKET_H
