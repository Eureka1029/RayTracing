#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <vector>
#include <limits>
#include <cstdlib>
// 常量

const double infinity = std::numeric_limits<double>::infinity(); //无穷
const double pi = 3.1415926535897932385; //pi

// 函数

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0; //角度转为弧度制
}
inline double random_double() {
    return std::rand() / (RAND_MAX + 1.0); // 返回数值处于[0,1)
}

inline double random_double(double min, double max) {
    return min + (max - min)*random_double(); //返回数值处于[min,max)
}


// Common Headers

#include "color.h"

#endif