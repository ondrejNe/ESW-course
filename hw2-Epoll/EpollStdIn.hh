#ifndef EPOLLSTDIN_H
#define EPOLLSTDIN_H

#include <stdint.h>
#include "EpollEntry.hh"

/**
 * @file EpollStdIn.hh
 * @brief Header file for EpollStdIn class.
 *
 * This file contains the definition of the EpollStdIn class, which represents
 * standard input that can be registered with an epoll instance.
 */

class EpollStdIn : public EpollEntry
{
public:
    EpollStdIn();
    bool handleEvent(uint32_t events);
};

#endif // EPOLLSTDIN_H
