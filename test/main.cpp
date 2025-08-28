#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>

#include "../include/RedisPublisher.h"

#include "../src/RedisSubscriber.cpp"

using namespace std;
using json = nlohmann::json;

int main()
{
    RedisSubscriber subscriber("localhost", 6379);
    while(true)
    {
        set <string> message = subscriber.listen();
        for(auto e : message) cout << e << " ";
        cout << "\n";
    }

}