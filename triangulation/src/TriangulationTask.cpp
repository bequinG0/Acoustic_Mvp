#include <iostream>

#include "../include/TriangulationTask.h"
#include "../include/RedisSubscriber.h"
#include "../include/Sensor.h"

using namespace std;

int time2int (string time)
{
    int hours = (time[0]-'0')*60*60*1000*10 + (time[1]-'0')*60*60*1000;
    int min = (time[3]-'0')*60*1000*10 + (time[4]-'0')*60*1000;
    int sec = (time[6]-'0')*1000*10 + (time[7]-'0')*1000;
    int ms = (time[9]-'0')*100 + (time[10]-'0')*10 + (time[11]-'0');
    int res = hours + min + sec + ms;
    return res;
}

TriangulationTask::TriangulationTask(vector <Sensor> sensors, vector <vector<string>> sensors_messages)
{
    int delta_time;
    for(auto message : sensors_messages)
    {
        vector <int> times;
        times.push_back(time2int(message[3]));
    }
    triangulator = Triangulator({sensors[0].x, sensors[0].y, delta_time});
}

void TriangulationTask::execute()
{
    
}