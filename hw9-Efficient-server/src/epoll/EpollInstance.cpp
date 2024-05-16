#include "EpollModel.hh"

void EpollInstance::registerEpollEntry(EpollEntry &e) const {
    epollLogger.debug("Register: %s", to_string(e.get_fd()));

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = e.get_events();
    ev.data.ptr = &e;

    if (epoll_ctl(this->fd, EPOLL_CTL_ADD, e.get_fd(), &ev) == -1) {
        throw runtime_error(string("epoll_ctl: ") + strerror(errno));
    }
}

void EpollInstance::unregisterEpollEntry(EpollEntry &e) const {
    epollLogger.debug("Unregister: %s", to_string(e.get_fd()));

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events = 0;
    ev.data.ptr = &e;

    if (epoll_ctl(this->fd, EPOLL_CTL_DEL, e.get_fd(), &ev) == -1) {
        throw runtime_error(string("epoll_ctl: ") + strerror(errno));
    }

    close(e.get_fd());
}

void EpollInstance::waitAndHandleEvents() {
    struct epoll_event events[EPOLL_MAX_EVENTS];

    int n = epoll_wait(this->fd, events, EPOLL_MAX_EVENTS, -1);

    epollLogger.debug("wait returned: " + to_string(n));

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
