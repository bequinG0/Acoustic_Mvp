#include <iostream>

#include "../include/TriangulationTask.h"
#include "../include/RedisSubscriber.h"

using namespace std;

TriangulationTask::TriangulationTask(string host, int port, string topic)
{
    RedisSubscriber subscriber(host, port, topic);
    eventsData = subscriber.listen();
}

void TriangulationTask::execute()
{
    
}