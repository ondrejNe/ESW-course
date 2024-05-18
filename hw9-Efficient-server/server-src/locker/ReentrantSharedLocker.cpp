#include "ReentrantSharedLocker.hh"

// Initialize thread-local storage
thread_local int ReentrantSharedLocker::sharedLockCount = 0;
thread_local int ReentrantSharedLocker::uniqueLockCount = 0;

void ReentrantSharedLocker::uniqueLock() {
#ifdef ENABLE_LOCKING
    if (uniqueLockCount == 0) {
        rwMutex.lock();
    }
    ++uniqueLockCount;
#endif
}

void ReentrantSharedLocker::uniqueUnlock() {
#ifdef ENABLE_LOCKING
    if (--uniqueLockCount == 0) {
        rwMutex.unlock();
    }
#endif
}

void ReentrantSharedLocker::sharedLock() {
#ifdef ENABLE_LOCKING
    if (sharedLockCount == 0) {
        rwMutex.lock_shared();
    }
    ++sharedLockCount;
#endif
}

void ReentrantSharedLocker::sharedUnlock() {
#ifdef ENABLE_LOCKING
    if (--sharedLockCount == 0) {
        rwMutex.unlock_shared();
    }
#endif
}
