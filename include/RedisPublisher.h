#include <iostream>

#include <hiredis/hiredis.h>

using namespace std;

class RedisPublisher
{
    private:
        redisContext* context;
    public:

        RedisPublisher(string host, int port, string channel_name)
        {
            context = redisConnect(host.c_str(), port);
            if(context == nullptr || (*context).err )
            {
                cout << "[ER] Ошибка подключения\n";
                exit(1);                
            }
            else cout << "[**] Вы подключились к redis\n";
        }

        void publish(string topic, string message)
        {
            string res = "PUBLISH " + topic + " " + message;
            redisReply* repl = (redisReply*)redisCommand(context, res.c_str());
            if(repl) freeReplyObject(repl);
        }
};