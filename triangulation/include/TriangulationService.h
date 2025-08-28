#include <iostream>
#include <hiredis/hiredis.h>
#include <vector>

#include "ThreadPool.h"
#include "RedisSubscriber.h"

using namespace std;

class TriangulationService
{
    private:
        RedisSubscriber sensorsSubscriber;
        ThreadPool theradPool;
        //sensors

        void processEventDataRecived()
        {

        }
        void processSensorsUpdate()
        {

        }

    public:
        
        TriangulationService();

        //??
};

