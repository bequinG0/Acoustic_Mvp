#include <iostream>
#include <cmath>
#include <tuple>

#include "../include/TriangulationTask.h"
#include "../include/RedisSubscriber.h"
#include "../include/SensorMessage.h"
#include "../include/Sensor.h"

using namespace std;

int time2int(const string& time) 
{
    Logger logger(".log");
    if (time.length() < 12)// Проверка длины
    {
        logger.addWriting("Time string too short", 'E');
        return 0;
    }
    else if (time[2] != ':' || time[5] != ':' || time[8] != ':')
    {   
        logger.addWriting("Invalid time format", 'E');
        return 0;
    }
    try 
    {
        int hours = (time[0]-'0')*10*60*60*1000 + (time[1]-'0')*60*60*1000;
        int min = (time[3]-'0')*10*60*1000 + (time[4]-'0')*60*1000;
        int sec = (time[6]-'0')*10*1000 + (time[7]-'0')*1000;
        int ms = (time[9]-'0')*100 + (time[10]-'0')*10 + (time[11]-'0');
        
        return hours + min + sec + ms;
    } 
    catch (...) 
    {
        logger.addWriting("Invalid characters in time", 'E');
        return 0;
    }
}

TriangulationTask::TriangulationTask(vector <Sensor> sensors, vector <SensorMessage> sensors_messages)
{ //переписать, добавить массив messages с которыми работаем, добавить массив датчиков с которыми работаем.
    int delta_time = 100, temp = 0; 
    vector <vector <string>> ch;
    temp = time2int(sensors_messages[sensors_messages.size()-1].timestump);
    for(int i=sensors_messages.size()-1; i>0; i--)
    {
        if(abs(time2int(sensors_messages[i].timestump) - temp) <= delta_time) ch.push_back({sensors_messages[i].timestump, sensors_messages[i].timestump});
        if(ch.size() == 3) break;
    }
    if(ch.size() == 1)
    {
        Sensor point;
        for(int i=0; i<sensors.size(); i++)
        {
            if(sensors[i].mac == ch[0][0]) { point.x = sensors[i].x; point.y=sensors[i].y; }
        }
        triangulator = Triangulator(make_tuple(point.x, point.y, 1));
    }
    else if(ch.size() == 2)
    {
        vector <Sensor> points;
        for(int i=0; i<sensors.size(); i++)
        {
            if(sensors[i].mac == ch[0][0]) points.push_back(Sensor(sensors[i].x, sensors[i].y));
            else if(sensors[i].mac == ch[1][0]) points.push_back(Sensor(sensors[i].x, sensors[i].y));
        }
        triangulator = Triangulator(make_tuple(points[0].x, points[0].y, 1),
        make_tuple(points[1].x, points[1].y, 1));
    }
    else if(ch.size() == 3)
    {
        vector <Sensor> points;
        for(int i=0; i<sensors.size(); i++)
        {
            if(sensors[i].mac == ch[0][0]) points.push_back(Sensor(sensors[i].x, sensors[i].y));
            else if(sensors[i].mac == ch[1][0]) points.push_back(Sensor(sensors[i].x, sensors[i].y));
            else if(sensors[i].mac == ch[2][0]) points.push_back(Sensor(sensors[i].x, sensors[i].y));
        }
        triangulator = Triangulator(make_tuple(points[0].x, points[0].y, 1),
        make_tuple(points[1].x, points[1].y, 1), make_tuple(points[2].x, points[2].y, 1));
    }
    else
    {
        triangulator = Triangulator();
    }
}

Point TriangulationTask::execute()
{
    return triangulator.combine();
}