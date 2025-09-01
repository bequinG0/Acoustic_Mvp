#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <condition_variable>

#include "../include/ThreadPool.h"
#include "../include/TriangulationTask.h"

using namespace std;

ThreadPool::ThreadPool(int n)
{
    count_of_threads = n; work_flag = true;
    for(int i=0; i<count_of_threads; i++) threads.emplace_back(&ThreadPool::worker, this);
}

ThreadPool::~ThreadPool()
{
    for(int i=0; i<threads.size(); i++) threads[i].join();
}

void ThreadPool::worker()
{
    while(work_flag)
    {   
        shared_ptr <Task> task;
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]{ return !tasks.empty() || !work_flag; });
        if(tasks.empty() && !work_flag) break;

        task = move(tasks.front());
        tasks.pop();
        lock.unlock();
        (*task).execute();
    }
}

void ThreadPool::addTask(shared_ptr <Task> task)
{
    lock_guard<mutex> lock(mtx);
    tasks.push(task);
    cv.notify_one();
}

void ThreadPool::finishAllThreads()
{
    for(int i=0; i<threads.size(); i++) threads[i].join();
}