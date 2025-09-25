#pragma once
#ifndef REDIS_SUBSCRIBER_H
#define REDIS_SUBSCRIBER_H

#include <iostream>
#include <set>
#include <vector>
#include <hiredis/hiredis.h>

#include "Logger.h"
#include "Sensor.h"
#include "SensorMessage.h"

using namespace std;

class RedisSubscriber
{
    private:
        redisContext* context;
        vector <string> topics;
    public:

        RedisSubscriber() {}
        RedisSubscriber(string host, int port);
        ~RedisSubscriber() {
            redisFree(context);
        }
        SensorMessage sensor_listen();
        void subscribe(string topic);
        vector <Sensor> updateTopics(RedisSubscriber &subscriber);
};

#endif //REDIS_SUBSCRIBER_H