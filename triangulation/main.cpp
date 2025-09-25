#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>

#include "TriangulationService.h"
#include "include/RedisPublisher.h"
#include "include/RedisSubscriber.h"
#include "include/ThreadPool.h"
#include "include/config.h"

using namespace std;
using namespace cfg;
using json = nlohmann::json;

int main()
{
   TriangulationService service;
   service.start();

   return 0;
}

