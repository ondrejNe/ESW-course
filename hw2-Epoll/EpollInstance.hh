/**
 * @file EpollInstance.hh
 * @brief Header file for EpollInstance class.
 *
 * This file contains the definition of the EpollInstance class, which is
 * responsible for managing the epoll instance and associated entries.
 */

#ifndef EPOLLINSTANCE_H
#define EPOLLINSTANCE_H

#include <stdint.h>
#include "EpollEntry.hh"

#define EPOLL_MAX_EVENTS 64

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
    void waitAndHandleEvents(void) const;

private:
    int fd; ///< File descriptor for the epoll instance.
};

#endif // EPOLLINSTANCE_H
