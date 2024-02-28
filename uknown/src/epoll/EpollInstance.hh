#include <iostream>
#include <mutex>
#include <unistd.h>
#include <sys/epoll.h>
#include <cstring>
#include <stdexcept>

#include "EpollEntry.hh"

#ifndef EPOLLINSTANCE_H
#define EPOLLINSTANCE_H

#define EPOLL_MAX_EVENTS 1024

using namespace std;

/**
 * @class EpollInstance
 * @brief Manages the epoll instance and associated entries.
 */
class EpollInstance
{
public:
    /**
     * @brief Constructor for EpollInstance. Initializes the epoll instance.
     */
    EpollInstance();
    
    /**
     * @brief Destructor for EpollInstance. Closes the epoll instance.
     */
    ~EpollInstance();

    /**
     * @brief Registers an EpollEntry with the epoll instance.
     *
     * @param e Reference to the EpollEntry to register.
     */
    void registerEpollEntry(EpollEntry &e) const;
    
    /**
     * @brief Unregisters an EpollEntry from the epoll instance.
     *
     * @param e Reference to the EpollEntry to unregister.
     */
    void unregisterEpollEntry(EpollEntry &e) const;
    
    /**
     * @brief Waits for and handles events from the epoll instance.
     */
    void waitAndHandleEvents();

    void set_fd(int i);
    int get_fd() const;

private:
    int fd; ///< File descriptor for the epoll instance.
};

#endif // EPOLLINSTANCE_H
