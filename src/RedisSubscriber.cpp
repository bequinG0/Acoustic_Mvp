#include <iostream>
#include <string>
#include <set>
#include <nlohmann/json.hpp>
#include <hiredis/hiredis.h>

#include "../include/RedisSubscriber.h"

using namespace std;
using json = nlohmann::json;
using js_err = json::parse_error;

set <string> parser(json data)
{
    set <string> result = {};
    result.insert(data["mac"]);
    result.insert(to_string(data["avg_volume"]));
    result.insert(data["class"]);
    result.insert(data["timestamp"]);
    result.insert(data["probs"]);
    return result;
}

template <typename T>
void output(vector <T> a)
{
    for (int i=0; i<a.size(); i++) cout << a[i] << " ";
    cout << "\n";
}

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
    freeReplyObject(channel_reply);
    json data = json::parse(message_str);
    for(auto e : data["sensors"]) topics.push_back(e["mac"]);
    for(auto e : topics)
    {
        string channel_name = "SUBSCRIBE " + e;
        redisReply* channel_reply = (redisReply*) redisCommand(context, channel_name.c_str());
        if(channel_reply == nullptr) 
        {
            cout << "[ER] Ошибка подписки\n";
        }
        else cout << "[**] Вы подписались на канал " + e + "\n";
        freeReplyObject(channel_reply);
    }
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
    try
    {
        json data = json::parse(message_str);
        
        set <string> result = parser(data);
        return result;
    }
    catch(js_err& err)
    {
        return {message_str};
    }        
}
