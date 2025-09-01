#include <iostream>
#include <hiredis/hiredis.h>

#include "../include/RedisPublisher.h"
#include "../include/Logger.h"

using namespace std;

RedisPublisher::RedisPublisher(string host, int port, string channel_name)
{
    Logger logger(".log");
    context = redisConnect(host.c_str(), port);
    if(context == nullptr || (*context).err ) logger.addWriting("Ошибка подключения", 'E');
    else logger.addWriting("Вы подключились к каналу", 'I');
}

void RedisPublisher::publish(string topic, string message)
{
    string res = "PUBLISH " + topic + " " + message;
    redisReply* repl = (redisReply*)redisCommand(context, res.c_str());
    if(repl) freeReplyObject(repl);
}


