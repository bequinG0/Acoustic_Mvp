#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <complex>
#include <cmath>
#include <fstream>
#include <utility>

#include "../include/Sensor.h"
#include "../include/RedisSubscriber.h"
#include "../include/ThreadPool.h"
#include "../include/Triangulator.h"

using namespace std;

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
    output<double>(x_n);
    cout << x_n.size() << "\n";

    //Реализуем алгоритм Дискретного преобразования Фурье

    vector<double> w, phase, amp;
    for(int n=0; n<x_n.size(); n++)
    {
        double temp = 0.5 * (1 - cos(2*M_PI*n/(N-1)));
        w.push_back(temp);
    }
    for(int k=0; k<N-1; k++)
    {
        complex <double> sum;
        sum.real(0); sum.imag(0);
        for(int n=0; n<=N-1; n++)
        {
            complex <double> eiphi;
            eiphi.real(cos(2*pi*n*k/N));
            eiphi.imag(-sin(2*pi*n*k/N));
            sum += x_n[n] * w[n] * eiphi;
        }
        X_n.push_back(sum);
        phase.push_back(arg(X_n[k]));
        amp.push_back(abs(X_n[k]));
    }

    //Находим искомую фазу по максимальной частоте
    pair <double, double> result;

    for(int i=0; i<X_n.size(); i++)
    {
        int temp = max_element(amp.begin(), amp.end()) - amp.begin();
        if(temp) result.first = phase[temp];
        result.second = N-temp;
    }

    return result;
}

pair <double, double> PointDeterminate(vector <vector <string>> sensors_messages, vector <Sensor> sensors)
{
    pair <double, double> cords;
    
    /*  Снимаем PCM-data с датчика, перед началом работы программы она лежит в sensors_messages 
        При написании кода, я считаю, что в массиве sensors находится ТРИ датчика по данным которых проводится триангуляция*/
    
    pair <double, double> one(sensors[0].x, sensors[0].y), two(sensors[1].x, sensors[1].y), three(sensors[2].x, sensors[2].y);

    // Находим параметры гиперболы с первых двух датчиков
    double R = sqrt(pow((one.first - two.second), 2) + pow((one.second - two.second), 2));
    sensors[0].freq_phase = PhaseDeterminate(sensors_messages[0][0]); // <-- это из расчёта что всё определено до вызова алгоритма
    sensors[1].freq_phase = PhaseDeterminate(sensors_messages[1][0]);
    sensors[2].freq_phase = PhaseDeterminate(sensors_messages[2][0]);
    double delta_t = (sensors[0].freq_phase.first - sensors[1].freq_phase.second)/(2*pi);

    return cords;
}

int main()
{
    cout << PhaseDeterminate("pcm_data_example.txt").first << " " << PhaseDeterminate("pcm_data_example.txt").second << "\n";
}

