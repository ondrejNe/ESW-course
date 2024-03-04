#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sys/epoll.h>

#include "EpollStdIn.hh"

/**
 * @file EpollStdIn.cpp
 * @brief Implementation of EpollStdIn class.
 *
 * This file contains the implementation of the EpollStdIn class, which
 * represents standard input that can be registered with an epoll instance. It
 * includes the implementation of the event handling for standard input.
 */

EpollStdIn::EpollStdIn()
{
    this->set_fd(STDIN_FILENO);
    this->set_events(EPOLLIN);
}

bool
EpollStdIn::handleEvent(uint32_t events)
{
    std::string line;
    if ((events & EPOLLERR) || (events & EPOLLHUP) || !(events & EPOLLIN)) {
        return false;
    } else {
        std::getline(std::cin, line);
        std::cout << "stdin line: " << line << std::endl;
        return true;
    }
}
