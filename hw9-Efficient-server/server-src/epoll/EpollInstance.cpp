
#include "EpollInstance.hh"

// Global variables -------------------------------------------------------------------------------
PrefixedLogger epollLogger = PrefixedLogger("[EPOLL INST]", true);

// Class definition -------------------------------------------------------------------------------
void EpollInstance::registerEpollEntry(std::unique_ptr<EpollEntry> e) {
    int fd = e->get_fd();
    epollLogger.debug("Register: %d", fd);

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
    epollLogger.debug("Unregister: %d", fd);

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = 0;
    ev.data.ptr = nullptr;

    if (epoll_ctl(this->fd, EPOLL_CTL_DEL, fd, &ev) == -1) {
        throw std::runtime_error(std::string("epoll_ctl: ") + strerror(errno));
    }
    epollLogger.debug("Unregistered: %d", fd);
    close(fd);
    epollLogger.debug("Closed: %d", fd);
    // Remove the entry from the map
    this->entries.erase(fd);
    epollLogger.warn("Erased %d", fd);
    for (const auto& entry : this->entries) {
        int key = entry.first;
        const EpollEntry* e = entry.second.get();
        epollLogger.debug("Key: %d, FD: %d", key, e->get_fd());
    }
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
