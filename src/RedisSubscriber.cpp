#include <iostream>
#include <string>
#include <set>
#include <nlohmann/json.hpp>
#include <hiredis/hiredis.h>

#include "../include/RedisSubscriber.h"

using namespace std;
using json = nlohmann::json;

string arr2str();

RedisSubscriber::RedisSubscriber(string host, int port)
{   
    context = redisConnect(host.c_str(), port);
    if(context == nullptr || (*context).err )
    {
        cout << "[ER] Ошибка подключения\n";
        exit(1);                
    }
    else cout << "[**] Вы подключились к redis\n";
    
    redisReply* channel_reply = (redisReply*) redisCommand(context, "SUBSCRIBE update_sensors");
    if(channel_reply == nullptr) 
    {
        cout << "[ER] Ошибка подписки\n";
    }
    else cout << "[**] Вы подписались на канал\n";
    freeReplyObject(channel_reply);
}
    

set <string> RedisSubscriber::listen()
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
    json data = json::parse(message_str);
    set <string> result;
    result.insert(data["mac"]);
    result.insert(data["timestamp"]);
    result.insert(data["class"]);
    result.insert(to_string(data["power"]));
    result.insert(to_string(data["probs"]));
    return result;     
}

vector <string> RedisSubscriber::updateTopics()
{
    vector <string> topics;
    redisReply* reply = (redisReply*)redisCommand(context, "PUBSUB CHANNELS");
    if((*reply).type == REDIS_REPLY_ARRAY)
    {
        for(int i=0; i<(*reply).elements; i++) topics.push_back((*(*reply).element[i]).str);
    }
    return topics;
}