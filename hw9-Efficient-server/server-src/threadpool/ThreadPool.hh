
#ifndef THREADPOOL_HH
#define THREADPOOL_HH

#include <bits/stdc++.h>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "Logger.hh"

//#define THREAD_LOGGER
#define ACTIVE_LOGGER_THREADPOOL true
using namespace std;

class ThreadPool {
private:
    bool                                    stop;
    deque<pair<function<void()>, int>>      tasks;
    vector<thread>                          threads;
    mutex                                   synchMutex;
    condition_variable                      synchCondition;
    PrefixedLogger                          threadpoolLogger;

public:
    ThreadPool(size_t numThreads) :
    stop(false),
    threadpoolLogger("[THREADPOOL]", ACTIVE_LOGGER_THREADPOOL) {
        for (size_t i = 0; i < numThreads; ++i) {
            // For logging purposes perform suspensions
            this_thread::sleep_for(chrono::milliseconds(1));

            threads.emplace_back([this] { // lambda function
#ifdef THREAD_LOGGER
                threadpoolLogger.info("Thread created");
#endif
                while (true) {
                    unique_lock<mutex> lock(synchMutex);
                    synchCondition.wait(lock, [this]() { return stop || !tasks.empty(); });

                    if (stop && tasks.empty()) return;

                    pair<function<void()>, int> task = move(tasks.front());
                    tasks.pop_front();;
                    lock.unlock();
#ifdef THREAD_LOGGER
                    threadpoolLogger.info("Task retrieved from queue FD%d", task.second);
#endif
                    task.first();
#ifdef THREAD_LOGGER
                    threadpoolLogger.info("Task executed FD%d", task.second);
#endif
                }
            });
        }
    }

    ~ThreadPool() {
        shutdown();
    }

    void waitAllThreads() {
        for (thread &thread : threads) {
            thread.join();
        }
    }

    void run(function<void()> task, int id) {
        {
            lock_guard<mutex> lock(synchMutex);
            tasks.push_back(make_pair(move(task), id));
#ifdef THREAD_LOGGER
            threadpoolLogger.info("Task added to the queue for FD%d", id);
#endif
        }
        synchCondition.notify_one();
    }

    void shutdown() {
        { // scoped lock
            lock_guard<mutex> lock(synchMutex);
            stop = true;
            synchCondition.notify_all();
        }

        for (thread &thread : threads) {
            thread.join();
        }
    }
};

#endif // THREADPOOL_HH
