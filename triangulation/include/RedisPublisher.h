#pragma once
#ifndef REDIS_PUBLISHER_H
#define REDIS_PUBLISHER_H

#include <iostream>
#include <hiredis/hiredis.h>

using namespace std;

class RedisPublisher
{
    private:
        redisContext* context;
    public:

        RedisPublisher(string host, int port, string channel_name);

        void publish(string topic, string message);
};

#endif //REDIS_PUBLISHER_H