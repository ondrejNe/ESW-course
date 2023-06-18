#include "ThreadPool.hh"

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back( [this]() { // lmabda function
            while (true) {
                function<void()> task;
                
                {
                    unique_lock<mutex> lock(synchMutex); // scoped lock
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

void ThreadPool::enqueue(function<void()> task)
{
    {
        lock_guard<mutex> lock(synchMutex); // scoped lock
        tasks.emplace(task);
    }
    condition.notify_one();
}

void ThreadPool::shutdown()
    {
        {
            lock_guard<mutex> lock(synchMutex); // scoped lock
            stop = true;
        }
        condition.notify_all();

        for (thread& thread : threads) {
            thread.join();
        }
    }

void ThreadPool::waitAllThreads() {
    for (thread &thread : threads) {
        thread.join();
    }
}
