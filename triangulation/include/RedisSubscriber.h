#pragma once
#ifndef REDIS_SUBSCRIBER_H
#define REDIS_SUBSCRIBER_H

#include <iostream>
#include <set>
#include <vector>
#include <hiredis/hiredis.h>

#include "../include/Logger.h"

using namespace std;

struct Sensor
{
    string mac, name;
    double x, y;

};

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
        set <string> sensor_listen();
        void subscribe(string topic);
        vector <Sensor> updateTopics(RedisSubscriber &subscriber);
};

#endif //REDIS_SUBSCRIBER_H