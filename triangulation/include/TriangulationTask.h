#pragma once
#ifndef TRIANGULATION_TASK_H
#define TRIANGULATION_TASK_H

#include "RedisPublisher.h"
#include "Triangulator.h"
#include <vector>
#include <utility>

#include "RedisSubscriber.h"
#include "Triangulator.h"
#include "SensorMessage.h"
#include "Sensor.h"

using namespace std;

class Task
{
    public:
        virtual void execute() = 0;

        virtual ~Task() = default;
};


class TriangulationTask : public Task
{
    private:
        Triangulator triangulator;
    public:

        TriangulationTask(vector <Sensor> sensors, vector <SensorMessage> sensors_messages);
        void execute();
};

#endif //TRIANGULATION_TASK_H