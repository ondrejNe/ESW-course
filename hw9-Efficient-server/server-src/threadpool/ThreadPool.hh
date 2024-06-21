
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

#define ACTIVE_LOGGER_THREADPOOL true
using namespace std;

class ThreadPool {
private:
    bool                            stop;
    deque<pair<function<void()>, int>>         tasks;
    vector<thread>                  threads;
    mutex                           synchMutex;
    condition_variable              synchCondition;
    PrefixedLogger                  threadpoolLogger;
    int                             counter;

public:
    ThreadPool(size_t numThreads) :
    stop(false),
    threadpoolLogger("[THREADPOOL]", ACTIVE_LOGGER_THREADPOOL),
    counter(0) {
        for (size_t i = 0; i < numThreads; ++i) {
            // For logging purposes perform suspensions
            this_thread::sleep_for(chrono::milliseconds(1));

            threads.emplace_back([this] { // lambda function
                threadpoolLogger.info("Thread created");
                while (true) {
                    unique_lock<mutex> lock(synchMutex);
                    synchCondition.wait(lock, [this]() { return stop || !tasks.empty(); });

                    if (stop && tasks.empty()) return;

                    pair<function<void()>, int> task = move(tasks.front());
                    tasks.pop_front();;
                    lock.unlock();

                    threadpoolLogger.info("Task retrieved from queue %d", task.second);
                    task.first();
                    threadpoolLogger.info("Task executed %d", task.second);
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

    void run(function<void()> task) {
        {
            lock_guard<mutex> lock(synchMutex);
            int id = counter++;
            tasks.push_back(make_pair(move(task), id));
            threadpoolLogger.info("Task added to the queue %d", id);
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
