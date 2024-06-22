
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

// Global variables -------------------------------------------------------------------------------

// Class definition -------------------------------------------------------------------------------
using namespace std;

class ThreadPool {
private:
    bool                                    stop;
    deque<pair<function<void()>, int>>      tasks;
    vector<thread>                          threads;
    mutex                                   synchMutex;
    condition_variable                      synchCondition;
public:
    ThreadPool(size_t numThreads);

    ~ThreadPool() {
        shutdown();
    }

    void waitAllThreads();

    void run(function<void()> task, int id);

    void shutdown();
};

#endif // THREADPOOL_HH
