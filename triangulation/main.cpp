#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>

#include "include/RedisPublisher.h"
#include "include/RedisSubscriber.h"
#include "include/ThreadPool.h"

using namespace std;
using json = nlohmann::json;

int main()
{
    RedisSubscriber listener("localhost", 6379), updater("localhost", 6379);
    vector <set <string>> sensors_messages;
    vector <Sensor> sensor_list;

    atomic <bool> running{true};
    mutex mtx;

    updater.subscribe("updated_sensors");
    sensor_list = updater.updateTopics(listener);
    
    ThreadPool task_pool(20);
    
    // прослушивание датчиков
    thread listen_thread([&]()
    {
        while (running) {
            set<string> message = listener.sensor_listen();
            {
                lock_guard<mutex> lock(mtx);
                sensors_messages.push_back(message);
            }
            this_thread::sleep_for(chrono::milliseconds(5));
        }
    });

    //обновление списка датчиков, обновление топиков
    thread update_thread([&]()
    {
        while (running)
        {
            vector<Sensor> current_sensors;
            {
                lock_guard<mutex> lock(mtx);
                current_sensors = sensor_list;
            }
            vector<Sensor> updated_list = updater.updateTopics(listener);
            {
                lock_guard<mutex> lock(mtx);
                sensor_list = updated_list;
            }
            
            this_thread::sleep_for(chrono::milliseconds(5));
        }
    });
    
    //Заполнение пула задач
    while (running) {
        try {
            vector<Sensor> current_sensors;
            vector<set<string>> current_messages;
            
            {
                lock_guard<mutex> lock(mtx);
                current_sensors = sensor_list;
                current_messages = sensors_messages;
            }
            
            shared_ptr<TriangulationTask> task = make_shared<TriangulationTask>(current_sensors, current_messages);
            task_pool.addTask(task);
            
            this_thread::sleep_for(chrono::milliseconds(10));
            
        } catch (const exception& e) {
            cerr << "Main thread error: " << e.what() << endl;
        }
    }

    running = false;
    listen_thread.join();
    update_thread.join();

}