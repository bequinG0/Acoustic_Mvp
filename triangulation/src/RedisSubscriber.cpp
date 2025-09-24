#include <iostream>
#include <string>
#include <set>
#include <nlohmann/json.hpp>
#include <hiredis/hiredis.h>

#include "../include/RedisSubscriber.h"
#include "../include/SensorMessage.h"

using namespace std;
using json = nlohmann::json;
using js_err = json::parse_error;

vector <string> parseJS(json data)
{
    vector <string> result = {};
    result.push_back(data["mac"]);
    result.push_back(to_string(data["avg_volume"]));
    result.push_back(data["class"]); // PCM_data
    result.push_back(data["timestamp"]);
    result.push_back(to_string(data["probs"])); // не надо
    return result;
}

template <typename T>
void output(vector <T> a)
{
    for (int i=0; i<a.size(); i++) cout << a[i] << " ";
    cout << "\n";
}

vector<string> csplit(string s, char element) {
    vector<string> str_hex;
    
    for(int i = 0; i < s.size(); i++) {
        if(s[i] == element && i + 3 < s.size() && s[i+1] == 'x') {
            // Нашли \x, теперь извлекаем два HEX символа
            string hex_byte;
            hex_byte += s[i+2];  // первая цифра
            hex_byte += s[i+3];  // вторая цифра
            str_hex.push_back(hex_byte);
            i += 3;  // пропускаем \x и две цифры
        }
    }
    return str_hex;
}

vector <int16_t> hex_to_dec(vector <string> a)
{
    vector <int16_t> result;

    for(int i=0; i<a.size(); i+=2)
    {
        int temp = 0;
        for(int j=0; j<a[i].size(); j++)
        {
            cout << a[i+1][j] << " " <<  2*a[i].size()-j-1 << " pows\n";
            cout << a[i][j] << " " << a[i].size()-j-1 << " pows\n";

            if(a[i+1][j] == 'A') temp += 10*pow(16, 2*a[i].size()-j-1);
            else if(a[i+1][j] == 'B') temp += 11*pow(16, 2*a[i].size()-j-1);
            else if(a[i+1][j] == 'C') temp += 12*pow(16, 2*a[i].size()-j-1);
            else if(a[i+1][j] == 'D') temp += 13*pow(16, 2*a[i].size()-j-1);
            else if(a[i+1][j] == 'E') temp += 14*pow(16, 2*a[i].size()-j-1);
            else if(a[i+1][j] == 'F') temp += 15*pow(16, 2*a[i].size()-j-1);
            else temp += (a[i+1][j] - '0')*pow(16, 2*a[i].size() - j -1);

            if(a[i][j] == 'A') temp += 10*pow(16, a[i].size()-j-1);
            else if(a[i][j] == 'B') temp += 11*pow(16, a[i].size()-j-1);
            else if(a[i][j] == 'C') temp += 12*pow(16, a[i].size()-j-1);
            else if(a[i][j] == 'D') temp += 13*pow(16, a[i].size()-j-1);
            else if(a[i][j] == 'E') temp += 14*pow(16, a[i].size()-j-1);
            else if(a[i][j] == 'F') temp += 15*pow(16, a[i].size()-j-1);
            else temp += (a[i][j] - '0')*pow(16, a[i].size()-j-1);
        }
        cout << temp << "\n";
        result.push_back(temp);
    }
    return result;
}

RedisSubscriber::RedisSubscriber(string host, int port)
{
    Logger logger(".log");
    context = redisConnect(host.c_str(), port);
    if(context == nullptr || (*context).err ) logger.addWriting("Ошибка подключения", 'E');               
    else logger.addWriting("Вы подключились к каналу ", 'I');
}

void RedisSubscriber::subscribe(string topic)
{
    Logger logger(".log");
    string channel_name = "SUBSCRIBE " + topic;
    redisReply* channel_reply = (redisReply*) redisCommand(context, channel_name.c_str());
    if(channel_reply == nullptr) logger.addWriting("Ошибка подписки", 'E');
    else logger.addWriting("Вы подписались на topic " + topic, 'I');
    freeReplyObject(channel_reply);
}

vector <Sensor> RedisSubscriber::updateTopics(RedisSubscriber &subscriber)
{
    Logger logger(".log");
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
        else logger.addWriting("error redis remote", 'E');
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

SensorMessage RedisSubscriber::sensor_listen()
{
    Logger logger1(".log");
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
    else logger1.addWriting("error redis remote", 'E');
    if(message_str.size() % 8 != 0 )
    {
        Logger logger(".log");
        logger.addWriting("error redis message format", 'E');
        return SensorMessage();
    }
    else
    {
        SensorMessage result;
        result.pcm_sound = hex_to_dec(csplit(message_str, '\\'));
        return result;
    }
            
}

