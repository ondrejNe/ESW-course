#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

#ifndef THREADPOOL_HH
#define THREADPOOL_HH

using namespace std;

class ThreadPool 
{
public:
    ThreadPool(size_t numThreads);

    void waitAllThreads();

    void enqueue(function<void()> task);

    void shutdown();

private:
    vector<thread> threads;
    queue<function<void()>> tasks;
    mutex synchMutex;
    condition_variable condition;
    bool stop;
};

#endif // THREADPOOL_HH
