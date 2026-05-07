#ifndef RAY_H
#define RAY_H

#include "vec3.h"

// ray 表示一条参数化光线：P(t) = origin + t * direction。
// 光线追踪的核心就是不断发射 ray，并查询它在世界中首先击中了什么。
class ray {
  public:
    // 默认构造函数：允许先声明 ray 变量，稍后由材质散射函数填入真实内容。
    ray() {}

    // 使用起点和方向创建光线；相机发出的主光线和材质散射光线都使用这个构造。
    ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}

    // 返回光线起点；命中检测需要用它计算射线与物体的相对位置。
    const point3& origin() const  { return orig; }

    // 返回光线方向；它决定光线前进方向，也参与反射、折射和求交计算。
    const vec3& direction() const { return dir; }

    // 根据参数 t 得到光线上的点；求出命中距离后，用它计算真实交点坐标。
    point3 at(double t) const {
        return orig + t*dir;
    }

  private:
    // 光线起点，通常是相机位置或上一次命中点。
    point3 orig;

    // 光线方向，不要求必须是单位向量；求交公式会按实际长度处理。
    vec3 dir;
};

  #endif
