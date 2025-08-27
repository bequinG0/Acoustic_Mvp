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
    vector <string> topics;

    topics.push_back("news_1");
    topics.push_back("news_2");

    RedisSubscriber subscriber("localhost", 6379);
    for(int i=0; i<3; i++)
    {
        set <string> message = subscriber.listen();
        for(auto e : message) cout << e << " ";
        cout << "\n";
    }

}