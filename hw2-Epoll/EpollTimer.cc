#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "EpollTimer.hh"

/**
 * @file EpollTimer.cpp
 * @brief Implementation of EpollTimer class.
 *
 * This file contains the implementation of the EpollTimer class, which
 * represents a timer that can be registered with an epoll instance. It
 * includes the implementation of the timer's creation, destruction, and event
 * handling.
 */

EpollTimer::EpollTimer(uint32_t timeMs)
{
    struct itimerspec its;
    memset(&its, 0, sizeof(its));

    this->set_fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK));
    if (this->get_fd() == -1) {
        throw std::runtime_error(
                std::string("timerfd_create: ") + std::strerror(errno));
    }

    its.it_interval.tv_sec = timeMs / 1000;
    its.it_interval.tv_nsec = (timeMs % 1000) * 1000000;
    its.it_value.tv_sec = timeMs / 1000;
    its.it_value.tv_nsec = (timeMs % 1000) * 1000000;

    if (timerfd_settime(this->get_fd(), 0, &its, NULL)) {
        throw std::runtime_error(
                std::string("timerfd_settime: ") + std::strerror(errno));
    }

    this->set_events(EPOLLIN);
}

EpollTimer::~EpollTimer()
{
    close(this->get_fd());
}

bool
EpollTimer::handleEvent(uint32_t events)
{
    uint64_t value;
    if ((events & EPOLLERR) || (events & EPOLLHUP) || !(events & EPOLLIN)) {
        return false;
    } else {
        auto len = read(this->get_fd(), &value, 8);
        (void)len;
        std::cout << "timer: " << this->get_fd() << std::endl;
        return true;
    }
}
