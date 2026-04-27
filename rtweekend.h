#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <vector>
#include <limits>


// 常量

const double infinity = std::numeric_limits<double>::infinity(); //正无穷
const double pi = 3.1415926535897932385; //pi

// 函数

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0; //角度转为弧度制
}

// Common Headers

#include "color.h"

#endif