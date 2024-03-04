#include <unistd.h>
#include <sys/epoll.h>
#include <cstring>
#include <stdexcept>
#include "EpollInstance.hh"

/**
 * @file EpollInstance.cpp
 * @brief Implementation of EpollInstance class.
 *
 * This file contains the implementation of the EpollInstance class, which is
 * responsible for managing the epoll instance and associated entries. It
 * includes the implementation of epoll entry registration, unregistration, and
 * event handling.
 */

EpollInstance::EpollInstance()
{
    /* Creates an epoll instance.  Returns an fd for the new instance.
   The "size" parameter is a hint specifying the number of file
   descriptors to be associated with the new instance.  The fd
   returned by epoll_create() should be closed with close().  */
   /* Same as epoll_create but with an FLAGS parameter.  The unused SIZE
   parameter has been dropped.  */
    this->fd = epoll_create1(0);
    if (this->fd == -1) {
        throw std::runtime_error(
                std::string("epoll_create1: ") + std::strerror(errno));
    }
}

EpollInstance::~EpollInstance()
{
    close(this->fd);
}

void
EpollInstance::registerEpollEntry(EpollEntry &e) const
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = e.get_events();
    ev.data.ptr = &e;

    if (epoll_ctl(this->fd, EPOLL_CTL_ADD, e.get_fd(), &ev) == -1) {
        throw std::runtime_error(
                std::string("epoll_ctl: ") + std::strerror(errno));
    }
}

void
EpollInstance::unregisterEpollEntry(EpollEntry &e) const
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = 0;
    ev.data.ptr = &e;

    if (epoll_ctl(this->fd, EPOLL_CTL_DEL, e.get_fd(), &ev) == -1) { // Check if the epoll_ctl function call to delete the entry fails.
    throw std::runtime_error( // If it fails, throw a runtime error with an error message containing the error details.
            std::string("epoll_ctl: ") + std::strerror(errno));
}

}

void
EpollInstance::waitAndHandleEvents(void) const
{
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int n = epoll_wait(this->fd, events, EPOLL_MAX_EVENTS, -1);
    if (n == -1) {
        throw std::runtime_error(
                std::string("epoll_wait: ") + std::strerror(errno));
    }
    for (int i = 0; i < n; i++) {
        EpollEntry *e = static_cast<EpollEntry *>(events[i].data.ptr);
        if (!e->handleEvent(events[i].events)) {
            this->unregisterEpollEntry(*e);
        }
    }
}
