#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <atomic>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>

#include "../include/RedisPublisher.h"

#include "../include/RedisSubscriber.h"

using namespace std;
using json = nlohmann::json;

vector <set <string>> listen(RedisSubscriber &subscriber,  vector <set <string>> &a)
{
    set <string> message;
    vector <set <string>> sensors_messages;
    message = subscriber.sensor_listen();
    sensors_messages.push_back(message);
    return sensors_messages;
}

vector <Sensor> update(RedisSubscriber &updater, RedisSubscriber &listener, vector <Sensor> &sensor_list)
{
    sensor_list = updater.updateTopics(listener);
    return sensor_list;
}

int main()
{
    RedisSubscriber listener("localhost", 6379), updater("localhost", 6379);
    vector <Sensor> sensor_list;
    updater.subscribe("updated_sensors");
    sensor_list = update(updater, listener, sensor_list);

    for(auto e : sensor_list) cout << e.mac << "\n";

    set <string> message;
    vector <set <string>> sensors_messages;
    message = listener.sensor_listen();
    sensors_messages.push_back(message);

    for(int i=0; i<sensors_messages.size(); i++)
    {
        for(auto e : sensors_messages[i]) cout << e << " ";
        cout << " ";
    }

}