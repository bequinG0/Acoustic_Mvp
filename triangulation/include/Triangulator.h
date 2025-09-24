#pragma once
#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <complex>
#include <cmath>
#include <tuple>
#include <utility>
#include <valarray>

#include "Sensor.h"
#include "Hyper.h"

using namespace std;
using Point = tuple<double, double, double>; // (x, y, s)

class Triangulator
{
    private:
        vector <Point> points;
        vector <Sensor> sensors;
        vector <vector <int16_t>> sensors_messages;

    public:

        void SetEventSelectionStrategy();

        Triangulator() {};
        Triangulator(const Point& p1);
        Triangulator(const Point& p1, const Point& p2);
        Triangulator(const Point& p1, const Point& p2, const Point& p3);

        vector<int16_t> readWavFile(const string& filename);
        double normalizePhase(double phase);
        pair<double, double> SpecialNewton(const Hyper& h1, const Hyper& h2);
        void fft(valarray<complex<double>>& x);
        double FindGlobalFreq(const vector<vector<int16_t>>& signals);
        vector<double> ConstsDeterminate(const vector<int16_t>& pcm_data);
        pair<double, double> PointDeterminate(vector<vector<int16_t>> sensors_messages, vector<Sensor> sensors);


        Point combine() const;
};

#endif //TRIANGULATOR_H