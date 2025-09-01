#pragma once
#ifndef TRIANGULATION_TASK_H
#define TRIANGULATION_TASK_H

#include "RedisPublisher.h"
#include "Triangulator.h"
#include <vector>

#include "RedisSubscriber.h"
#include "Triangulator.h"

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
        RedisSubscriber subscriber;
        Triangulator triangulator;
        string eventsData;
    public:

        TriangulationTask(string host, int port, string topic);
        void execute();
};

#endif //TRIANGULATION_TASK_H