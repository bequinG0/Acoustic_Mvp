#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <hiredis/hiredis.h>

#include "../include/RedisPublisher.h"
#include "../include/RedisSubscriber.h"

using namespace std;

int main()
{
    RedisSubscriber subscriber("localhost", 6379, "news");
    while(true)
    {
        char button = getchar();
        if(button == '1') break;

        string message = subscriber.listen();
        cout << "[**] Получено новое сообщение: " << message << "\n";
    }
    ~RedisPublisher subscriber;
}