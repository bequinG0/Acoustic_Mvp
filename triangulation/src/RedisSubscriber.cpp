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
    result.insert(to_string(data["probs"]));
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
    Logger logger(".log");
    context = redisConnect(host.c_str(), port);
    if(context == nullptr || (*context).err ) logger.addWriting("Ошибка подключения", 'E');               
    else logger.addWriting("Вы подключились к каналу", 'I');
}

void RedisSubscriber::subscribe(string topic)
{
    Logger logger(".log");
    string channel_name = "SUBSCRIBE " + topic;
    redisReply* channel_reply = (redisReply*) redisCommand(context, channel_name.c_str());
    if(channel_reply == nullptr) logger.addWriting("Ошибка подписки", 'E');
    else logger.addWriting("Вы подписались на канал", 'I');
    freeReplyObject(channel_reply);
}

vector <Sensor> RedisSubscriber::updateTopics(RedisSubscriber &subscriber)
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
        vector <Sensor> sensors;
        for(auto e : data["sensors"]) 
        {
            topics.push_back(e["mac"]);
            Sensor temp;
            temp.mac = e["mac"]; temp.name = e["name"];
            temp.x = e["x"]; temp.y = e["y"];
            sensors.push_back(temp);
        }
        for(auto e : topics) subscriber.subscribe(e);
        return sensors;
    }
    catch(js_err& err)
    {
        return vector <Sensor>();
    }
}

set <string> RedisSubscriber::sensor_listen()
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
        return {""};
    }
            
}
