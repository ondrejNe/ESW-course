
#include "ThreadPool.hh"

// Global variables -------------------------------------------------------------------------------
//#define THREAD_LOGGER
PrefixedLogger threadLogger = PrefixedLogger("[THREADPOOL]", true);
#define THREAD_STATS_LOGGER
PrefixedLogger threadStatsLogger = PrefixedLogger("[THREADPOOL STATS]", true);

#ifdef THREAD_STATS_LOGGER
uint64_t taskCounter = 0;
uint64_t maxTasks = 0;
#endif

// Class definition -------------------------------------------------------------------------------
ThreadPool::ThreadPool(size_t numThreads) :
        stop(false)
{
    for (size_t i = 0; i < numThreads; ++i) {
        // For logging purposes perform suspensions
        this_thread::sleep_for(chrono::milliseconds(1));

        threads.emplace_back([this] { // lambda function
#ifdef THREAD_LOGGER
            threadLogger.info("Thread created");
#endif
            while (true) {
                unique_lock<mutex> lock(synchMutex);
                synchCondition.wait(lock, [this]() { return stop || !tasks.empty(); });

                if (stop && tasks.empty()) return;

                pair<function<void()>, int> task = move(tasks.front());
                tasks.pop_front();;
                lock.unlock();
#ifdef THREAD_LOGGER
                threadLogger.info("Task retrieved from queue FD%d", task.second);
#endif
                task.first();
#ifdef THREAD_LOGGER
                threadLogger.info("Task executed FD%d", task.second);
#endif
            }
        });
    }
}


void ThreadPool::waitAllThreads() {
    for (thread &thread : threads) {
        thread.join();
    }
}

void ThreadPool::run(function<void()> task, int id) {
    {
        lock_guard<mutex> lock(synchMutex);
        tasks.push_back(make_pair(move(task), id));
#ifdef THREAD_LOGGER
        threadLogger.info("Task added to the queue for FD%d", id);
#endif
#ifdef THREAD_STATS_LOGGER
        taskCounter++;
        if (tasks.size() > maxTasks) {
            maxTasks = tasks.size();
        }
        threadStatsLogger.info("Task counter: %lu max: %lu", taskCounter, maxTasks);
#endif
    }
    synchCondition.notify_one();
}

void ThreadPool::shutdown() {
    { // scoped lock
        lock_guard<mutex> lock(synchMutex);
        stop = true;
        synchCondition.notify_all();
    }

    for (thread &thread : threads) {
        thread.join();
    }
}
