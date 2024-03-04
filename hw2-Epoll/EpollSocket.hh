#ifndef EPOLL_SOCKET_H
#define EPOLL_SOCKET_H

#include <stdint.h>
#include "EpollEntry.hh"
#include "EpollConnection.hh"
#include "EpollInstance.hh"

class EpollSocket : public EpollEntry
{
public:
    // Constructor creates the listening socket
    EpollSocket(uint16_t port, EpollInstance &ep);
    
    // Destructor closes the listening socket
    ~EpollSocket();
    
    // Accept connections and create epoll connection entries
    bool handleEvent(uint32_t events);

private:
    EpollInstance &epollInstance; // Reference to the epoll instance
};

#endif // EPOLL_SOCKET_H
