#include "EpollEntry.hh"

/**
 * @file EpollEntry.cpp
 * @brief Implementation of EpollEntry class.
 *
 * This file contains the implementation of the EpollEntry class, which serves
 * as a base class for objects that can be registered with an epoll instance.
 * It includes the implementation of file descriptor and event management.
 */

void
EpollEntry::set_fd(int i)
{
    this->fd = i;
}

int
EpollEntry::get_fd() const
{
    return this->fd;
}

void
EpollEntry::set_events(uint32_t i)
{
    this->events = i;
}

uint32_t
EpollEntry::get_events() const
{
    return this->events;
}
