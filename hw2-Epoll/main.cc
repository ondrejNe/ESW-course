#include <iostream>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "EpollInstance.hh"
#include "EpollStdIn.hh"
#include "EpollTimer.hh"
#include "EpollSocket.hh"

using namespace std;

int
main(void)
{
    try {
        // A touch of dry British humour
        cout << "The TCP server is starting. Keep calm and carry on." << endl;

        EpollInstance ep;
        EpollTimer tim1(1000);
        EpollTimer tim2(1500);
        EpollStdIn sin;
        EpollSocket serv(12345, ep);

        ep.registerEpollEntry(tim1);
        ep.registerEpollEntry(tim2);
        ep.registerEpollEntry(sin);
        ep.registerEpollEntry(serv);

        while (1) {
            ep.waitAndHandleEvents();
        }

        ep.unregisterEpollEntry(tim1);
        ep.unregisterEpollEntry(tim2);
        ep.unregisterEpollEntry(sin);
        ep.unregisterEpollEntry(serv);

    } catch (const std::exception &e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
