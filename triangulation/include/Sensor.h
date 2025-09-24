#pragma once
#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>
#include <string>
#include <utility>

using namespace std;

struct Sensor {
    string mac, name;
    double x, y;
    vector <double> c;
    
    Sensor() {}
    Sensor(double X, double Y) : x(X), y(Y) {}
};

#endif //SENSOR_H