


#ifndef AABB_H
#define AABB_H

#include"interval.h"
#include"vec3.h"
#include"ray.h"

class aabb {
  public:
    // AABB 在三个坐标轴上的投影区间。
    interval x, y, z;

    aabb() {} // 默认 AABB 为空，因为 interval 的默认构造就是空区间。

    aabb(const interval& x, const interval& y, const interval& z)
      : x(x), y(y), z(z) 
      {
        pad_to_minimums();
      }

    aabb(const point3& a, const point3& b) {
        // 将 a 和 b 视为包围盒的两个对角顶点；下面分别取每个轴上的较小值和较大值，
        // 因此调用者不需要保证 a 一定是最小角点、b 一定是最大角点。

        x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);
        pad_to_minimums();
    }

    //合并包围盒
    aabb(const aabb& bbox0, const aabb& bbox1){
        x = interval(bbox0.x, bbox1.x);
        y = interval(bbox0.y, bbox1.y);
        z = interval(bbox0.z, bbox1.z);
    }

    const interval& axis_interval(int n) const {
        // 根据轴编号返回对应的投影区间：0 -> x，1 -> y，2 -> z。
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool hit(const ray& r, interval ray_t) const {
        // 使用 slab 方法检测光线是否穿过 AABB。
        // ray_t 表示当前允许的光线参数范围，并会被每个轴的相交区间逐步收窄。
        const point3& ray_orig = r.origin();
        const vec3&   ray_dir  = r.direction();

        for (int axis = 0; axis < 3; axis++) {
            const interval& ax = axis_interval(axis);
            const double adinv = 1.0 / ray_dir[axis];

            // 计算光线与当前轴两侧边界平面相交时的参数 t。
            auto t0 = (ax.min - ray_orig[axis]) * adinv;
            auto t1 = (ax.max - ray_orig[axis]) * adinv;

            // 若光线在该轴方向为负，t0 和 t1 的先后顺序会反过来；
            // 用较大的进入时间更新下界，用较小的离开时间更新上界。
            if (t0 < t1) {
                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            } else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            // 三个轴对应的 t 区间必须存在公共交集；一旦交集为空，就不可能命中 AABB。
            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;

    }
    int longest_axis() const {
        //返回最长轴
        if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;
    }

    static const aabb empty, universe;
    void pad_to_minimums(){
        // 确保 AABB 每个轴向的宽度不小于最小值，必要时向两侧扩展。

        double delta = 0.0001;
        if(x.size() < delta) x = x.expand(delta);
        if(y.size() < delta) y = y.expand(delta);
        if(z.size() < delta) z = z.expand(delta);
    }

};

const aabb aabb::empty  = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

aabb operator+(const aabb& bbox, const vec3& offset) {
    return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

aabb operator+(const vec3& offset, const aabb& bbox) {
    return bbox + offset;
}

#endif
