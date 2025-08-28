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