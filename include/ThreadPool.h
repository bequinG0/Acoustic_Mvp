#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <condition_variable>

#include <TriangulationService.h>
#include <TriangulationTask.h>

using namespace std;

class ThreadPool
{
    private:
        queue <TriangulationTask> tasks;
        vector <thread> threads;
        atomic <bool> work_flag;
        int count_of_threads;
        mutex mtx;
        condition_variable cv;

    public:

        ThreadPool(int n);
        ~ThreadPool();
        void start();
        void worker();
        void addTask(TriangulationTask &task);
        void finishAllThreads();
};

