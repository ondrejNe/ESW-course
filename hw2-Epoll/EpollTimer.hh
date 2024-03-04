#ifndef EPOLLTIMERIN_H
#define EPOLLTIMERIN_H

#include <stdint.h>
#include "EpollEntry.hh"

/**
 * @file EpollTimer.hh
 * @brief Header file for EpollTimer class.
 *
 * This file contains the definition of the EpollTimer class, which represents
 * a timer that can be registered with an epoll instance.
 */

class EpollTimer : public EpollEntry
{
public:
    EpollTimer(uint32_t timeMs);
    ~EpollTimer();
    bool handleEvent(uint32_t events);
};

#endif // EPOLLTIMERIN_H
