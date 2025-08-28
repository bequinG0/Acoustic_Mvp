#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <condition_variable>

#include <TriangulationTask.h>

using namespace std;

class ThreadPool
{
    private:
        queue <Task> tasks;
        vector <thread> threads;
        atomic <bool> work_flag;
        int count_of_threads;
        mutex mtx;
        condition_variable cv;

    public:

        ThreadPool(int n);
        ~ThreadPool();
        void worker();
        void addTask(Task &task);
        void finishAllThreads();
};

