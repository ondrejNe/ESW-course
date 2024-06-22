
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
using namespace std;

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
class ThreadPool {
private:
    bool                                    stop;
    deque<pair<function<void()>, int>>      tasks;
    vector<thread>                          threads;
    mutex                                   synchMutex;
    condition_variable                      synchCondition;
public:
    ThreadPool(size_t numThreads) :
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
            threadLogger.info("Task added to the queue for FD%d", id);
#endif
#ifdef THREAD_STATS_LOGGER
            taskCounter++;
            threadStatsLogger.info("Task counter: " + to_string(taskCounter));
            if (tasks.size() > maxTasks) {
                maxTasks = tasks.size();
                threadStatsLogger.warn("Max tasks: " + to_string(maxTasks));
            }
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
