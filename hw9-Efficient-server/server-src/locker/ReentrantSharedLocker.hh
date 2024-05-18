#ifndef REENTRANTSHAREDLOCKER_HH
#define REENTRANTSHAREDLOCKER_HH

#include <shared_mutex>
#include <thread>

class ReentrantSharedLocker {
private:
    std::shared_mutex rwMutex;
    thread_local static int sharedLockCount;
    thread_local static int uniqueLockCount;

public:
    ReentrantSharedLocker() = default;

    void uniqueLock();
    void uniqueUnlock();
    void sharedLock();
    void sharedUnlock();
};

#endif // REENTRANTSHAREDLOCKER_HH
