#ifndef INTERVAL_H
#define INTERVAL_H
#include "rtweekend.h"

// interval 表示一段数值范围，主要用来限制光线参数 t 的有效区间。
// 在命中检测中，它能避免记录相机后方的交点，也能帮助找到离相机最近的物体。
class interval {
  public:
    // 区间下界和上界；通常用于描述光线从 t=min 到 t=max 的可见范围。
    double min, max;

    // 默认构造为空区间；min > max 表示没有任何数会落在里面。
    interval() : min(+infinity), max(-infinity) {}

    // 构造指定范围的区间；调用者可用它限制测试范围，例如 interval(0.001, infinity)。
    interval(double min, double max) : min(min), max(max) {}

    // 返回区间长度；可用于判断范围大小，虽然当前项目主要使用 contains/surrounds。
    double size() const {
        return max - min;
    }

    // 闭区间判断：允许 x 等于边界，适合普通范围包含测试。
    bool contains(double x) const {
        return min <= x && x <= max;
    }

    // 开区间判断：不允许 x 等于边界，命中检测用它避免边界处的自相交问题。
    bool surrounds(double x) const {
        return min < x && x < max;
    }

    // 将 x 限制到区间内；颜色输出时用它把浮点颜色压到合法强度范围。
    double clamp(double x) const {
        if(x < min) return min;
        if(x > max) return max;
        return x;
    }

    // 常用区间常量：empty 表示空范围，universe 表示所有实数范围。
    static const interval empty, universe;
};

// 空区间：作为“不存在有效范围”的哨兵值。
const interval interval::empty    = interval(+infinity, -infinity);

// 全区间：作为“不限制范围”的默认工具值。
const interval interval::universe = interval(-infinity, +infinity);

#endif
