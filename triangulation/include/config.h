#pragma once
#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>
using namespace std;

namespace cfg
{
    const string localhost = "localhost";
    const int lo_port = 6379;
    const string update_channel = "update_sensors";
    const int count_of_pool = 20;
    const int port = NULL;
}

#endif //CONFIG_H
