#pragma once
#ifndef TRIANGULATION_SERVICE_H
#define TRIANGULATION_SERVICE_H

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

#endif //TRIANGULATION_SERVICE_H