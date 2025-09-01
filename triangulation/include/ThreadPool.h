#pragma once
#ifndef TREAD_POOL_H
#define TREAD_POOL_H

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <condition_variable>

#include "TriangulationTask.h"
#include "Sensor.h"

using namespace std;

class ThreadPool
{
    private:
        queue <shared_ptr <Task>> tasks;
        vector <thread> threads;
        atomic <bool> work_flag;
        int count_of_threads;
        mutex mtx;
        condition_variable cv;

    public:

        ThreadPool(int n);
        ~ThreadPool();
        void worker();
        void addTask(shared_ptr<Task> task);
        void finishAllThreads();
};

#endif //TREAD_POOL_H
