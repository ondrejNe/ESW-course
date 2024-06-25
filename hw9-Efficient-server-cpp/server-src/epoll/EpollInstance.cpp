
#include "EpollInstance.hh"

#define EPOLL_MAX_EVENTS 2048

// Global variables -------------------------------------------------------------------------------
//#define EPOLL_LOGGER
PrefixedLogger epollLogger = PrefixedLogger("[EPOLL INST]", true);

// Class definition -------------------------------------------------------------------------------
void EpollInstance::registerEpollEntry(std::unique_ptr<EpollEntry> e) {
    int fd = e->get_fd();
#ifdef EPOLL_LOGGER
    epollLogger.debug("Register: [FD%d]", fd);
#endif
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = e->get_events();
    ev.data.ptr = e.get();

    if (epoll_ctl(this->fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        throw std::runtime_error(std::string("epoll_ctl: ") + strerror(errno));
    }

    // Store the unique_ptr in the map
    this->entries[fd] = std::move(e);
}

void EpollInstance::unregisterEpollEntry(int fd) {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = 0;
    ev.data.ptr = nullptr;

    if (epoll_ctl(this->fd, EPOLL_CTL_DEL, fd, &ev) == -1) {
        throw std::runtime_error(std::string("epoll_ctl: ") + strerror(errno));
    }
#ifdef EPOLL_LOGGER
    epollLogger.debug("Unregistered [FD%d]", fd);
#endif
    // Remove the entry from the map
    this->entries.erase(fd);
#ifdef EPOLL_LOGGER
    epollLogger.warn("Erased [FD%d]", fd);
#endif
    close(fd);
#ifdef EPOLL_LOGGER
    epollLogger.debug("Closed [FD%d]", fd);
    for (const auto& entry : this->entries) {
        const EpollEntry* e = entry.second.get();
        epollLogger.debug("Alive [FD%d]", e->get_fd());
    }
#endif
}

void EpollInstance::waitAndHandleEvents() {
    struct epoll_event events[EPOLL_MAX_EVENTS];

    int n = epoll_wait(this->fd, events, EPOLL_MAX_EVENTS, -1);

    if (n == -1) {
        throw std::runtime_error(std::string("epoll_wait: ") + strerror(errno));
    }

    for (int i = 0; i < n; i++) {
        EpollEntry *e = static_cast<EpollEntry *>(events[i].data.ptr);
        if (e->handleEvent(events[i].events) == false) {
            this->unregisterEpollEntry(e->get_fd());
        } else {
            if (epoll_ctl(this->fd, EPOLL_CTL_MOD, e->get_fd(), &events[i]) == -1) {
                throw std::runtime_error(std::string("epoll_ctl: ") + strerror(errno));
            }
        }
    }
}
