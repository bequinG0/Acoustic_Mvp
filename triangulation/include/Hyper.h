#pragma once
#ifndef HYPER_H
#define HYPER_H

#include <iostream>

using namespace std;

struct Hyper {
    double x, y;
    double a, b, alpha;
    int orient;
    
    Hyper() {}
    Hyper(double X, double Y, double A, double B, double Alp, int O) : 
        x(X), y(Y), a(A), b(B), alpha(Alp), orient(O) {}
    
    void getvalues()
    { cout << a << " " << b << " " << alpha << " " << orient << " " << x << " " << y << "\n";}
};

#endif //HYPER_H
