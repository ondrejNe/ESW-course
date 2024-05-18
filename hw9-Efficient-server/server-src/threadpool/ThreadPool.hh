
#ifndef THREADPOOL_HH
#define THREADPOOL_HH

#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>
#include <string>
#include <bits/stdc++.h>
#include <functional>
#include <chrono>

#include "Logger.hh"

using namespace std;

class ThreadPool {
private:
    bool stop;
    mutex synchMutex;
    vector<thread> threads;
    condition_variable condition;
    queue<function<void()>> tasks;
    PrefixedLogger threadpoolLogger = PrefixedLogger("[THREADPOOL]", DEBUG);
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back([this, i]() { // lambda function

                // Thread ID logging
                thread::id this_id = this_thread::get_id();
                size_t hash_id = hash<thread::id>{}(this_id);
                threadpoolLogger.info("Thread (%d) created with ID: %d", i, hash_id);
                // For logging purposes perform suspensions
                this_thread::sleep_for(chrono::milliseconds(1));

                while (true) {
                    function<void()> task;

                    { // scoped lock
                        unique_lock<mutex> lock(synchMutex);
                        condition.wait(lock, [this]() { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) return;

                        task = move(tasks.front());
                        tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    void waitAllThreads() {
        for (thread &thread : threads) {
            thread.join();
        }
    }

    void enqueue(function<void()> task) {
        { // scoped lock
            lock_guard<mutex> lock(synchMutex);
            tasks.emplace(task);
        }
        condition.notify_one();
    }

    void shutdown() {
        { // scoped lock
            lock_guard<mutex> lock(synchMutex);
            stop = true;
        }
        condition.notify_all();

        for (thread &thread : threads) {
            thread.join();
        }
    }
};

#endif // THREADPOOL_HH
