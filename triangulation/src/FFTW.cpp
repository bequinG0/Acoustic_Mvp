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
    double x, y;
    double a, b, alpha;
    int orient;

    Hyper(){}
    Hyper(double X, double Y, double A, double B, double Alp, int O)
    {   
        x = X; y = Y; a = A; b = B; alpha = Alp; orient = O;
    }
};

pair <double, double> SpeicalNewton(Hyper h1, Hyper h2)
{
    pair<double, double> result;
    double t1 = 0.5, t2 = 0.5;
    const double tolerance = 1e-6;
    const int max_iterations = 100; // бывает такое, что алгоритм Ньютона для систем нелинейных уравнений плохо сходить ->> решение

    for (int iter = 0; iter < max_iterations; iter++) 
    {
        double x1_local = h1.x + h1.orient * h1.a * cosh(t1); // парметризация в локальной СК
        double y1_local = h1.y + h1.b * sinh(t1);
        double x1 = x1_local * cos(h1.alpha) - y1_local * sin(h1.alpha) + h1.x; // переход в глобальную СК
        double y1 = x1_local * sin(h1.alpha) + y1_local * cos(h1.alpha) + h1.y;
        
        double x2_local = h2.x + h2.orient * h2.a * cosh(t2);
        double y2_local = h2.y + h2.b * sinh(t2);
        double x2 = x2_local * cos(h2.alpha) - y2_local * sin(h2.alpha) + h2.x;
        double y2 = x2_local * sin(h2.alpha) + y2_local * cos(h2.alpha) + h2.y;
        
        double f1 = x1 - x2;
        double f2 = y1 - y2;
        
        if (abs(f1) < tolerance && abs(f2) < tolerance) {
            result = make_pair(x1, y1);
            break;
        }
        
        double dt = 1e-6;
        
        double x1p_local = h1.orient * h1.a * cosh(t1+dt);
        double y1p_local = h1.b * sinh(t1+dt);
        double x1p = x1p_local * cos(h1.alpha) - y1p_local * sin(h1.alpha) + h1.x;
        double y1p = x1p_local * sin(h1.alpha) + y1p_local * cos(h1.alpha) + h1.y;
        
        double dx1_dt1 = (x1p - x1) / dt;
        double dy1_dt1 = (y1p - y1) / dt;
        
        double x2p_local = h2.x + h2.orient * h2.a * cosh(t2+dt);
        double y2p_local = h2. y + h2.b * sinh(t2+dt);
        double x2p = x2p_local * cos(h2.alpha) - y2p_local * sin(h2.alpha) + h2.x;
        double y2p = x2p_local * sin(h2.alpha) + y2p_local * cos(h2.alpha) + h2.y;
        
        double dx2_dt2 = (x2p - x2) / dt;
        double dy2_dt2 = (y2p - y2) / dt;
        
        double J[2][2] = { // Вычисления определителя матрицы Якоби, для итерационного метода
            {dx1_dt1, -dx2_dt2},
            {dy1_dt1, -dy2_dt2}
        };
        
        double det = J[0][0] * J[1][1] - J[0][1] * J[1][0];
        
        if (abs(det) < 1e-12) {
            t1 -= 0.01 * f1;
            t2 -= 0.01 * f2;
        } else {
            double invJ[2][2] = {
                {J[1][1]/det, -J[0][1]/det},
                {-J[1][0]/det, J[0][0]/det}
            };
            
            double dt1 = -(invJ[0][0] * f1 + invJ[0][1] * f2);
            double dt2 = -(invJ[1][0] * f1 + invJ[1][1] * f2);
            
            t1 += dt1;
            t2 += dt2;
        }
    }

    double x1_local = h1.orient * h1.a * cosh(t1);
    double y1_local = h1.b * sinh(t1);
    result = make_pair(
        x1_local * cos(h1.alpha) - y1_local * sin(h1.alpha) + h1.x,
        x1_local * sin(h1.alpha) + y1_local * cos(h1.alpha) + h1.y
    );

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
    double R_1 = sqrt(pow((one.first - two.first),2) + pow((one.second - two.second), 2));
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
    Hyper hyper1(0.5*(point1.x + point2.x), 0.5*(point1.y + point2.y), a, b, alpha, orientation);

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
    Hyper hyper2(0.5*(point1.x + point3.x), 0.5*(point1.y + point3.y), a, b, alpha, orientation);

    cords = SpeicalNewton(hyper1, hyper2);
    
    return cords;
}

int main()
{
    cout << PhaseDeterminate("pcm_data_example.txt").first << " " << PhaseDeterminate("pcm_data_example.txt").second << "\n";
}

