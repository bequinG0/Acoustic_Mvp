#include <iostream>
#include <set>
#include <vector>
#include <hiredis/hiredis.h>

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
        set <string> listen();
        vector <string> updateTopics();
};