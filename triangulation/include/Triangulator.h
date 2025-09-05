#pragma once
#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H

#include <iostream>
#include <vector>
#include <tuple>

#include "Sensor.h"
#include "EventSelectionStrategy.h"

using namespace std;
using Point = tuple<double, double, double>; // (x, y, s)

class Triangulator
{
    private:
        vector <Point> points;

    public:
        EventSelectionStrategy selectionStrategy;

        void SetEventSelectionStrategy();

        Triangulator() {};
        Triangulator(const Point& p1);
        Triangulator(const Point& p1, const Point& p2);
        Triangulator(const Point& p1, const Point& p2, const Point& p3);
        
        Point combine() const;
};

#endif //TRIANGULATOR_H