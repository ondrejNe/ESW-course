#ifndef EPOLL_CONNECTION_H
#define EPOLL_CONNECTION_H

#include <stdint.h>
#include "EpollEntry.hh"

class EpollConnection : public EpollEntry
{
public:
    // A proper constructor for an accepted connection
    EpollConnection(int fd);
    
    // Destructor closes the connection's file descriptor
    ~EpollConnection();
    
    // Handle incoming data or errors for the connection
    bool handleEvent(uint32_t events);

private:
    // Read data from the connection and return the length
    void readData();

    // Buffer for received data
    std::string receivedData_;
};

#endif // EPOLL_CONNECTION_H
