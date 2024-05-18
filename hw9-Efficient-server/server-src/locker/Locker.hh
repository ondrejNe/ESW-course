#ifndef LOCKER_HH
#define LOCKER_HH

#include <shared_mutex>

class SharedLocker {
private:
    std::shared_mutex rwMutex;
    std::unique_lock<std::shared_mutex> uniqueLockable;
    std::shared_lock<std::shared_mutex> sharedLockable;

public:
    SharedLocker() : uniqueLockable(rwMutex, std::defer_lock), sharedLockable(rwMutex, std::defer_lock) {}

    void uniqueLock() {
#ifdef ENABLE_LOCKING
        uniqueLockable.lock();
#endif
    }

    void uniqueUnlock() {
#ifdef ENABLE_LOCKING
        uniqueLockable.unlock();
#endif
    }

    void sharedLock() {
#ifdef ENABLE_LOCKING
        sharedLockable.lock();
#endif
    }

    void sharedUnlock() {
#ifdef ENABLE_LOCKING
        sharedLockable.unlock();
#endif
    }

};
#endif //LOCKER_HH
