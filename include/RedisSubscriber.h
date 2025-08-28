#include <iostream>
#include <set>
#include <vector>
#include <hiredis/hiredis.h>

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