#pragma once
#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>
#include <string>
#include <utility>

using namespace std;

struct Sensor
{
    string mac, name;
    double x, y;
    pair <double, double> freq_phase;
};

#endif //SENSOR_H