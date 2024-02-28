#include "EpollEntry.hh"

void EpollEntry::set_fd(int i) {
    this->fd = i;
}

int EpollEntry::get_fd() const {
    return this->fd;
}

void EpollEntry::set_events(uint32_t i) {
    this->events = i;
}

uint32_t EpollEntry::get_events() const {
    return this->events;
}
