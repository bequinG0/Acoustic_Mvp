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
const double pi = 3.14159265358979323846264438327950288;
const int sound_speed = 343;

template <typename for_out>
void output(vector <for_out> a)
{
    for(int i=0; i<a.size(); i++) cout << a[i] << " ";
    cout << " ";
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
            eiphi.real(cos(2*pi*n*k/N));
            eiphi.imag(-sin(2*pi*n*k/N));
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

    double delta_t1 = (sensors[0].freq_phase.first - sensors[1].freq_phase.first)/(2*pi*sensors[0].freq_phase.second);
    double delta_t2 = (sensors[2].freq_phase.first - sensors[2].freq_phase.first)/(2*pi*sensors[1].freq_phase.second);
        cout << sensors[0].freq_phase.second << " " << sensors[1].freq_phase.second << " " << sensors[2].freq_phase.second << "\n";

    // Построение первой гиперболы
    double R = sqrt(pow((one.first - two.second), 2) + pow((one.second - two.second), 2));
    double r1 = abs(R + delta_t1 * sound_speed)*0.5;
    double r2 = abs(R - delta_t2 * sound_speed)*0.5; //r2 < r1 всегда
    /*  Находим параметры гиперболы с первых двух датчиков
        Для начала необходимо повернуть СК чтобы найти гиперболу в перенсной СК, а затем вернуться в исходную*/
    Sensor point1(sensors[0].x, sensors[0].y), point2(sensors[1].x, sensors[1].y);
    double alpha = atan((point1.y - point2.y)/(point1.x - point2.x)) - pi/2;
    double b = point1.y + point1.x*(point2.y - point1.y)/(point1.x -point2.x);
    point1.y -= b; point2.y -=b;
    
    nc::NdArray<double> v1 = nc::NdArray<double>({2, 1}); 
    v1(0,0) = point1.x;
    v1(1,0) = point1.y;
    nc::NdArray<double> v2 = nc::NdArray<double>({2, 1}); 
    v2(0,0) = point2.x;
    v2(1,0) = point2.y;
    nc::NdArray<double> spin = nc::NdArray<double>({2, 2});
    spin(0, 0) = cos(alpha); spin(0, 1) = -sin(alpha);
    spin(1, 0) = sin(alpha); spin(1, 1) = cos(alpha);
    v1 = nc::dot(spin, v1); v2 = nc::dot(spin, v2);
    spin = transpose(spin);
    cout << v1(0, 0) << " " << v1(1, 0) << "\n";
    cout << v2(0, 0) << " " << v2(1, 0) << "\n";

    return cords;
}

int main()
{
    cout << PhaseDeterminate("pcm_data_example.txt").first << " " << PhaseDeterminate("pcm_data_example.txt").second << "\n";
}

