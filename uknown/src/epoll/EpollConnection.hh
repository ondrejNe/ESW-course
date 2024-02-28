#ifndef EPOLL_CONNECTION_H
#define EPOLL_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cstring> 
#include <cerrno>  
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
// Epoll
#include "EpollEntry.hh"
#include "EpollInstance.hh"
// Protobuf
#include "protobuf/scheme.pb.h"
// DataGrid
#include "data/DataGrid.hh"
// Threadpool
#include "threadpool/ThreadPool.hh"

using namespace std;

class EpollConnection : public EpollEntry
{
public:
    // A proper constructor for an accepted connection
    EpollConnection(int fd, DataGrid &dataGrid, ThreadPool &resourcePool);
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
    DataGrid &dataGrid;
    ThreadPool &resourcePool;
    // Single message variables
    bool messageInProgress;
    char *messageBuffer;
    int  inProgerssMessageSize;
    int  inProgerssMessageRead;
};

#endif // EPOLL_CONNECTION_H
