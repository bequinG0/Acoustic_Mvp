#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <stdexcept>

#include "../include/Triangulator.h"

using namespace std;

Triangulator::Triangulator(const Point& p1) {
    points.push_back(p1);
}

Triangulator::Triangulator(const Point& p1, const Point& p2) {
    points.push_back(p1);
    points.push_back(p2);
}

Triangulator::Triangulator(const Point& p1, const Point& p2, const Point& p3) {
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
}

Point Triangulator::combine() const {
    double s_max = 120;
    size_t n = points.size();
    if (n == 1) {
        auto [x, y, s] = points[0];
        return {x, y, 1};
    }
    if (n == 2) {
        const auto& [x1, y1, s1] = points[0];
        const auto& [x2, y2, s2] = points[1];
        std::vector<double> s_val {s1, s2};
        double s_min = *std::min_element(begin(s_val), end(s_val))-1;
        double w1 = (s1-s_min) / (s_max-s_min);
        double w2 = (s2-s_min) / (s_max-s_min);
        double w_sum = w1 + w2;
        if (w_sum == 0) {
            return {0,0,0};
        }
        double x =  (w1 * x1 + w2 * x2)/w_sum;
        double y =  (w1 * y1 + w2 * y2)/w_sum;
        return {x, y, 2};
    }
    if (n == 3) {
        const auto& [x1, y1, s1] = points[0];
        const auto& [x2, y2, s2] = points[1];
        const auto& [x3, y3, s3] = points[2];
        std::vector<double> s_val {s1, s2, s3};
        double s_min = *std::min_element(begin(s_val), end(s_val))-1;
        double w1 = (s1-s_min) / (s_max-s_min);
        double w2 = (s2-s_min) / (s_max-s_min);
        double w3 = (s3-s_min) / (s_max-s_min);
        double w_sum = w1 + w2 + w3;
        if (w_sum == 0) {
            return {0,0,0};
        }
        double x =  (w1 * x1 + w2 * x2 + w3 * x3)/w_sum;
        double y =  (w1 * y1 + w2 * y2 + w3 * y3)/w_sum;
        // std::cout << "triang:" << std::endl;
        // std::cout << s1 << ";" << s2 << ";" << s3 << std::endl;
        // std::cout << x1 << ";" << y1 << std::endl;
        // std::cout << x2 << ";" << y2 << std::endl;
        // std::cout << x3 << ";" << y3 << std::endl;
        std::cout << x << ";" << y << std::endl;
        return {x, y, 3};
    }
    throw std::runtime_error("Invalid number of points");
}
