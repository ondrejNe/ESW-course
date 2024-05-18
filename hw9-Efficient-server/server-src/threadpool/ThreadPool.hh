
#ifndef THREADPOOL_HH
#define THREADPOOL_HH

#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>

using namespace std;

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back([this]() { // lambda function
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

private:
    bool stop;
    mutex synchMutex;
    vector<thread> threads;
    condition_variable condition;
    queue<function<void()>> tasks;
};

#endif // THREADPOOL_HH
