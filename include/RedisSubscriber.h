#include <iostream>
#include <hiredis/hiredis.h>

using namespace std;

class RedisSubscriber
{
    private:
        redisContext* context;
        redisReply* channel_reply;
    public:

        RedisSubscriber(string host, int port, string channel_name);
        
        ~RedisSubscriber() {
            redisFree(context);
            freeReplyObject(channel_reply);
        }

        string listen();
};