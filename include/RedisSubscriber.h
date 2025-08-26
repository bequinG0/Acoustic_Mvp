#include <iostream>
#include <hiredis/hiredis.h>

using namespace std;

class RedisSubscriber
{
    private:
        redisContext* context;
        redisReply* channel_reply;
    public:

        RedisSubscriber(string host, int port, string channel_name)
        {
            context = redisConnect(host.c_str(), port);
            if(context == nullptr || (*context).err )
            {
                cout << "[ER] Ошибка подключения\n";
                exit(1);                
            }
            else cout << "[**] Вы подключились к redis\n";
            channel_name = "SUBSCRIBE " + channel_name;
            channel_reply = (redisReply*) redisCommand(context, channel_name.c_str());
            if(channel_reply == nullptr) 
            {
                cout << "[ER] Ошибка подписки\n";
            }
            else cout << "[**] Вы подписались на канал\n";
        }

        ~RedisSubscriber() {
            redisFree(context);
            freeReplyObject(channel_reply);
        }

        string listen()
        {
            while(true)
            {
                string message_str = "";
                redisReply* message_repl = nullptr;
                if(redisGetReply(context, (void**)&message_repl) == REDIS_OK) 
                {
                    if ((*message_repl).type == REDIS_REPLY_ARRAY && (*message_repl).elements == 3)
                    {
                        string action = (*(*message_repl).element[0]).str,
                        topic_name = (*(*message_repl).element[1]).str;
                        message_str = (*(*message_repl).element[2]).str;

                    }
                    freeReplyObject(message_repl);
                }
                return message_str;
            }
            return "ERR\n";
        }
};