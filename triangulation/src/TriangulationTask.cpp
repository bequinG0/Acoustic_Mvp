#include <iostream>
#include <cmath>
#include <tuple>

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
    int delta_time = 50, temp = 0, ch = 0;
    temp = time2int(sensors_messages[sensors_messages.size()-1][3]);
    for(int i=sensors_messages.size()-1; i>0; i--)
    {
        if(abs(time2int(sensors_messages[i][3]) - temp) <= delta_time) ch++;
    }
    if(ch == 3)
    {
        triangulator = Triangulator(make_tuple(sensors[sensors.size()-1].x, sensors[sensors.size()-1].y, 1));
    }
    else if(ch == 2)
    {
        triangulator = Triangulator(make_tuple(sensors[sensors.size()-1].x, sensors[sensors.size()-1].y, 1),
        make_tuple(sensors[sensors.size()-2].x, sensors[sensors.size()-2].y, 1));
    }
    else if(ch == 1)
    {
        triangulator = Triangulator(make_tuple(sensors[sensors.size()-1].x, sensors[sensors.size()-1].y, 1),
        make_tuple(sensors[sensors.size()-2].x, sensors[sensors.size()-2].y, 1),
        make_tuple(sensors[sensors.size()-1].x, sensors[sensors.size()-1].y, 1));
    }
    else
    {
        triangulator = Triangulator();
    }
}

void TriangulationTask::execute()
{
    triangulator.combine();
}