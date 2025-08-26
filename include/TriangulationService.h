#include <iostream>
#include <hiredis/hiredis.h>
#include <vector>

#include "ThreadPool.h"
#include "RedisSubscriber.h"
#include "EventDataExtractor.h"

using namespace std;

class TriangulationService
{
    private:
        RedisSubscriber sensorsSubscriber;
        EventDataExtractor eventDataExtractor;
        ThreadPool theradPool;
        //sensors

        void processEventDataRecived()
        {

        }
        void processSensorsUpdate()
        {

        }

    public:
        
        TriangulationService() {}

        //??
};

