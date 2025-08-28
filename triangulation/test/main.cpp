#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#include <atomic>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>

#include "../include/RedisPublisher.h"

#include "../src/RedisSubscriber.cpp"

using namespace std;
using json = nlohmann::json;

int main()
{
    RedisSubscriber subscriber("localhost", 6379);
    

}