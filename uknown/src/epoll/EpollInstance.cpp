#include "EpollInstance.hh"

// #define DEBUG_EPOLL
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
        throw runtime_error(string("epoll_create1: ") + strerror(errno));
    }
}

EpollInstance::~EpollInstance() {
    close(this->fd);
}

void EpollInstance::set_fd(int i) {
    this->fd = i;
}

int EpollInstance::get_fd() const {
    return this->fd;
}

void EpollInstance::registerEpollEntry(EpollEntry &e) const {
    #ifdef DEBUG_EPOLL
    cout << "[EPOLL " << this->fd << "]  Register: " << e.get_fd() << endl;
    #endif
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = e.get_events();
    ev.data.ptr = &e;

    if (epoll_ctl(this->fd, EPOLL_CTL_ADD, e.get_fd(), &ev) == -1) {
        throw runtime_error(string("epoll_ctl: ") + strerror(errno));
    }
}

void EpollInstance::unregisterEpollEntry(EpollEntry &e) const {
    #ifdef DEBUG_EPOLL
    cout << "[EPOLL " << this->fd << "]  Unregister: " << e.get_fd() << endl;
    #endif
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = 0;
    ev.data.ptr = &e;

    if (epoll_ctl(this->fd, EPOLL_CTL_DEL, e.get_fd(), &ev) == -1) { 
        throw runtime_error(string("epoll_ctl: ") + strerror(errno));
    }

    close(e.get_fd());
}

void EpollInstance::waitAndHandleEvents()
{
    struct epoll_event events[EPOLL_MAX_EVENTS];

    int n = epoll_wait(this->fd, events, EPOLL_MAX_EVENTS, -1);

    #ifdef DEBUG_EPOLL
    cout << "[EPOLL " << this->fd << "]  wait returned: " << n << endl;
    #endif
    if (n == -1) {
        throw runtime_error(string("epoll_wait: ") + strerror(errno));
    }

    for (int i = 0; i < n; i++) {
        EpollEntry *e = static_cast<EpollEntry *>(events[i].data.ptr);
        if (!e->handleEvent(events[i].events)) {
            this->unregisterEpollEntry(*e);
        } else {
            epoll_ctl(this->fd, EPOLL_CTL_MOD, e->get_fd(), &events[i]);
        }
    }
}
