#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <complex>
#include <cmath>
#include <fstream>
#include <utility>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>

#include "NumCpp.hpp"
#include "../include/Sensor.h"
#include "../include/RedisSubscriber.h"
#include "../include/ThreadPool.h"
#include "../include/Triangulator.h"

using namespace std;
namespace nc = nc;

const int N=50;
const int sound_speed = 343;

template <typename for_out>
void output(vector <for_out> a)
{
    for(int i=0; i<a.size(); i++) cout << a[i] << " ";
    cout << " ";
}

struct Hyper
{
    double a, b, alpha;
    int o;

    Hyper() {}
    Hyper(double A, double B, double phi, int O)
    { a = A; b = B; alpha = phi; o = O; }
};

pair <double, double> SpecialBisection(Hyper h1, Hyper h2)
{
    pair <double, double> result;  
    double left = -20, right = 20, t = 0;
    double y_1 = -t * sin(h1.alpha) + h1.o * sqrt(h1.b)*cos(h1.alpha) *sqrt(pow(t, 2)/pow(h1.a, 2) + 1),
    y_2 = -t*sin(h2.alpha) + h2.o * sqrt(h2.b) * cos(h2.alpha) * sqrt(pow(t, 2)/pow(h1.a, 2) + 1),
    x_1 = t*cos(h1.alpha) + h1.o * sqrt(h1.b) * sin(h1.alpha) * sqrt(pow(t, 2)/pow(h1.a, 2) + 1),
    x_2 = t*cos(h2.alpha) + h2.o * sqrt(h1.b) * sin(h1.alpha) * sqrt(pow(t, 2)/pow(h2.a, 2) + 1);

    while(abs(y_1 - y_2) > 0.01)
    {
        t = 0.5 * (left + right);
        y_1 = -t * sin(h1.alpha) + h1.o * sqrt(h1.b)*cos(h1.alpha) *sqrt(pow(t, 2)/pow(h1.a, 2) + 1);
        y_2 = -t*sin(h2.alpha) + h2.o * sqrt(h2.b) * cos(h2.alpha) * sqrt(pow(t, 2)/pow(h1.a, 2) + 1);
        if(y_1 - y_2 > 0) right = t;
        else if(y_1 - y_2 < 0) left = t;
    }
    y_1 = -t * sin(h1.alpha) + h1.o * sqrt(h1.b)*cos(h1.alpha) *sqrt(pow(t, 2)/pow(h1.a, 2) + 1); 
    result.second = y_1;
    left = -20; right = 20; t = 0;
    while(abs(x_1 - x_2) > 0.01)
    {
        t = 0.5 * (left + right);
        x_1 = t*cos(h1.alpha) + h1.o * sqrt(h1.b) * sin(h1.alpha) * sqrt(pow(t, 2)/pow(h1.a, 2) + 1);
        x_2 = t*cos(h2.alpha) + h2.o * sqrt(h1.b) * sin(h1.alpha) * sqrt(pow(t, 2)/pow(h2.a, 2) + 1);
        if(x_1 - x_2 > 0) right = t;
        else if(x_1 - x_2 < 0) left = t;
    }
    result.first = x_1;
    return result;
}

pair <double, double> PhaseDeterminate(string pcm_data)
{
    vector <double> x_n;
    vector <complex <double>> X_n;

    //Получаем данные каким-либо образом, например с редиса

    ifstream fin;
    fin.open(pcm_data);

    double value;
    vector <int> res;
    while(fin >> value) x_n.push_back(value);
    fin.close();

    //Реализуем алгоритм Дискретного преобразования Фурье

    vector<double> w, phase, amp;
    for(int n=0; n<x_n.size(); n++)
    {
        double temp = 0.5 * (1 - cos(2*M_PI*n/(N-1)));
        w.push_back(temp);
    }
    for(int k=0; k<x_n.size(); k++)
    {
        complex <double> sum;
        sum.real(0); sum.imag(0);
        for(int n=0; n<x_n.size(); n++)
        {
            complex <double> eiphi;
            eiphi.real(cos(2*M_PI*n*k/N));
            eiphi.imag(-sin(2*M_PI*n*k/N));
            sum += x_n[n] * w[n] * eiphi;
        }
        X_n.push_back(sum);
        phase.push_back(abs(arg(X_n[k])));
        amp.push_back(abs(X_n[k]));
    }

    //Находим искомую фазу по максимальной частоте
    pair <double, double> result;

    int temp = max_element(amp.begin(), amp.end()) - amp.begin();
    if(temp) result.first = phase[temp];
    result.second = temp;

    return result;
}

pair <double, double> PointDeterminate(vector <vector <string>> sensors_messages, vector <Sensor> sensors)
{
    pair <double, double> cords;
    
    /*  Снимаем PCM-data с датчика, перед началом работы программы она лежит в sensors_messages 
        При написании кода, я считаю, что в массиве sensors находится ТРИ датчика по данным которых проводится триангуляция*/
    
    pair <double, double> one(sensors[0].x, sensors[0].y), two(sensors[1].x, sensors[1].y), three(sensors[2].x, sensors[2].y);

    sensors[0].freq_phase = PhaseDeterminate(sensors_messages[0][3]); // <-- это из расчёта что всё определено до вызова алгоритма
    sensors[1].freq_phase = PhaseDeterminate(sensors_messages[1][3]);
    sensors[2].freq_phase = PhaseDeterminate(sensors_messages[2][3]);

    if((sensors[0].freq_phase.second != sensors[1].freq_phase.second)
    || (sensors[1].freq_phase.second != sensors[2].freq_phase.second) 
    || (sensors[2].freq_phase.second != sensors[0].freq_phase.second)) //  <-- Нет уверенности в том, что на максимум амплитуды наблюдается на одинаковых бинах
    {
        cout << sensors[0].freq_phase.second << " " << sensors[1].freq_phase.second << " " << sensors[2].freq_phase.second << "\n";
        if(sensors[0].freq_phase.second > sensors[1].freq_phase.second
        || sensors[0].freq_phase.second > sensors[2].freq_phase.second) sensors[0].freq_phase.second = N - sensors[0].freq_phase.second;
        else if(sensors[0].freq_phase.second < sensors[1].freq_phase.second
        || sensors[1].freq_phase.second > sensors[2].freq_phase.second) sensors[1].freq_phase.second = N - sensors[1].freq_phase.second;
        else if(sensors[2].freq_phase.second > sensors[0].freq_phase.second
        ||sensors[2].freq_phase.second > sensors[1].freq_phase.second) sensors[2].freq_phase.second = N - sensors[2].freq_phase.second;
    } // Для того чтобы убедиться, что с бинами всё ОК, нужно проводить тесты

    // Убедитесь, что разность времен вычисляется корректно
    double delta_t1 = (sensors[0].freq_phase.first - sensors[1].freq_phase.first) / (2 * M_PI * sensors[0].freq_phase.second);
    double delta_t2 = (sensors[0].freq_phase.first - sensors[2].freq_phase.first) / (2 * M_PI * sensors[0].freq_phase.second);
    cout << sensors[0].freq_phase.second << " " << sensors[1].freq_phase.second << " " << sensors[2].freq_phase.second << "\n";

    // Нахождение параметров первой гиперболы =========================================================================
    double R_1 = sqrt(pow((one.first - two.second), 2) + pow((one.second - two.second), 2));
    double r1 = abs(R_1 + delta_t1 * sound_speed)*0.5;
    double r2 = abs(R_1 - delta_t2 * sound_speed)*0.5; //r2 < r1 всегда
    /*  Находим параметры гиперболы с первых двух датчиков
        Для начала необходимо повернуть СК чтобы найти гиперболу в переносной СК, а затем вернуться в исходную*/
    Sensor point1(sensors[0].x, sensors[0].y), point2(sensors[1].x, sensors[1].y);
    double alpha = atan((point1.y - point2.y)/(point1.x - point2.x)) - M_PI/2;
    double p = point1.y + point1.x*(point2.y - point1.y)/(point1.x -point2.x);
    point1.y -= p; point2.y -=p;
    double a = R_1/2 - r1;
    double b = sqrt(pow(R_1, 2)/4 - pow(R_1/2 - r1, 2));
    int orientation = 0;
    if(sensors[0].y > sensors[1].y)
    {
        if(sensors_messages[0] > sensors_messages[1]) orientation = 1;
        else orientation = -1;
    }
    else
    {
        if(sensors_messages[0] < sensors_messages[1]) orientation = 1;
        else orientation = -1;
    }
    Hyper hyper1(a, b, alpha, orientation);

    //Нахождение параметров второй гиперболы =========================================================================
    double R_2 = sqrt(pow(one.first - three.first, 2) + pow(one.second - three.second, 2));
    r1 = abs(R_2 + delta_t2 * sound_speed)*0.5;
    r2 = abs(R_2 + delta_t2 * sound_speed)*0.5;
    Sensor point3(sensors[0].x, sensors[0].y), point4(sensors[2].x, sensors[2].y);
    alpha = atan((point3.y - point3.y)/(point4.x - point4.x)) - M_PI/2;
    p = point3.y + point3.x*(point4.y - point3.y)/(point3.x -point4.x);
    point3.y -= p; point4.y -=p;
    a = R_2/2 - r1;
    b = sqrt(pow(R_2, 2)/4 - pow(R_2/2 - r1, 2));
    orientation = 0;
    if(sensors[0].y > sensors[2].y)
    {
        if(sensors_messages[0] > sensors_messages[2]) orientation = 1;
        else orientation = -1;
    }
    else
    {
        if(sensors_messages[0] < sensors_messages[2]) orientation = 1;
        else orientation = -1;
    }
    Hyper hyper2(a, b, alpha, orientation);

    cords = SpecialBisection(hyper1, hyper2);

    /*
    nc::NdArray<double> vec_point1 = nc::NdArray<double>({2, 1}); 
    vec_point1(0,0) = point1.x;
    vec_point1(1,0) = point1.y;
    nc::NdArray<double> vec_point2 = nc::NdArray<double>({2, 1}); 
    vec_point2(0,0) = point2.x;
    vec_point2(1,0) = point2.y;
    nc::NdArray<double> spin = nc::NdArray<double>({2, 2});
    spin(0, 0) = cos(alpha); spin(0, 1) = -sin(alpha);
    spin(1, 0) = sin(alpha); spin(1, 1) = cos(alpha);
    vec_point1 = nc::dot(spin, vec_point1); vec_point2 = nc::dot(spin, vec_point2);
    spin = transpose(spin);
    */
    
    return cords;
}

int main()
{
    cout << PhaseDeterminate("pcm_data_example.txt").first << " " << PhaseDeterminate("pcm_data_example.txt").second << "\n";
}

