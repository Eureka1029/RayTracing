#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <vector>
#include <limits>
#include <cstdlib>
// 项目公共基础头文件：集中放置全局常量和小工具函数，避免每个模块重复定义。
// 这些内容会被向量、区间、相机等模块共同使用，是整个光线追踪程序的通用依赖。

// 正无穷：用于表示“没有上界”的距离区间，帮助命中检测排除或接收任意远的物体。
const double infinity = std::numeric_limits<double>::infinity();

// 圆周率：用于角度和弧度转换，相机视场角计算依赖它。
const double pi = 3.1415926535897932385;

// 将角度转换为弧度；三角函数使用弧度，因此相机 vfov 需要先经过这个函数处理。
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// 生成 [0,1) 的随机浮点数；用于像素抗锯齿采样、材质随机散射和随机场景生成。
inline double random_double() {
    return std::rand() / (RAND_MAX + 1.0);
}

// 生成 [min,max) 的随机浮点数；让调用者能指定随机范围，例如随机颜色、粗糙度和球体位置。
inline double random_double(double min, double max) {
    return min + (max - min)*random_double();
}


#endif
