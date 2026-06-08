#ifndef QUAD_H
#define QUAD_H

#include "hittable.h"
// box() 会返回一个 hittable_list，用 6 个 quad 组合成长方体。
#include "hittable_list.h"
class quad : public hittable {
  public:
    quad(const point3& Q, const vec3& u, const vec3& v, std::shared_ptr<material> mat)
      : Q(Q), u(u), v(v), mat(mat)
    {
        auto n = cross(u,v);
        normal = unit_vector(n);
        D = dot(normal,Q);
        w = n / dot(n, n);
        set_bounding_box();
    }

    virtual void set_bounding_box() {
        // 通过两条对角线的包围盒合并出四个顶点的整体包围盒。
        auto bbox_diagonal1 = aabb(Q, Q + u + v);
        auto bbox_diagonal2 = aabb(Q + u, Q + v);
        bbox = aabb(bbox_diagonal1, bbox_diagonal2);
    }

    aabb bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
      auto denom = dot(normal, r.direction());

      // 分母接近 0 时，光线方向几乎与平面平行，无法得到有效交点。
      if (std::fabs(denom) < 1e-8)
          return false;

      // 计算光线与平面的交点参数 t；若不在允许区间内，则忽略这次命中。
      auto t = (D - dot(normal, r.origin())) / denom;
      if (!ray_t.contains(t))
          return false;
      // 使用平面坐标判断交点是否落在四边形内部。
      auto intersection = r.at(t);
      vec3 planar_hitpt_vector = intersection - Q;
      auto alpha = dot(w, cross(planar_hitpt_vector, v));
      auto beta = dot(w, cross(u, planar_hitpt_vector));

      if (!is_interior(alpha, beta, rec))
          return false;

      // 光线命中这个二维形状，补全命中记录并返回 true。
      rec.t = t;
      rec.p = intersection;
      rec.mat = mat;
      rec.set_face_normal(r, normal);

      return true;
    }

    virtual bool is_interior(double a, double b, hit_record& rec) const {
      interval unit_interval = interval(0,1);
      if(!unit_interval.contains(a) || !unit_interval.contains(b)){
        return false;
      }
      rec.u = a;
      rec.v = b;
      return true;
    }

  private:
    point3 Q;
    vec3 u, v;
    std::shared_ptr<material> mat;
    aabb bbox;
    double D;
    vec3 normal;
    vec3 w;
};
inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat)
{
    // 用两个对角顶点 a、b 定义一个轴对齐长方体，并返回它的 6 个面。
    // 返回类型是 hittable_list，是因为长方体本身不是单个基础图元，
    // 而是由 front/right/back/left/top/bottom 六个 quad 共同组成。
    auto sides = make_shared<hittable_list>();

    // 调用者传入的 a、b 不一定按最小角和最大角排序。
    // 这里先整理出每个坐标轴上的 min/max，保证后续构造面的方向稳定。
    auto min = point3(std::fmin(a.x(),b.x()), std::fmin(a.y(),b.y()), std::fmin(a.z(),b.z()));
    auto max = point3(std::fmax(a.x(),b.x()), std::fmax(a.y(),b.y()), std::fmax(a.z(),b.z()));

    // 三个边向量分别表示长方体在 x/y/z 方向上的尺寸。
    // 每个 quad 由一个起点 Q 和两条边向量定义，因此下面会复用 dx/dy/dz
    // 或它们的反方向，拼出朝向正确的六个面。
    auto dx = vec3(max.x() - min.x(), 0, 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    // 每一面都选一个角点作为 Q，再给出沿该面的两条边。
    // 边向量顺序会影响 quad 的法线方向，下面的排列让法线整体朝向盒子外侧。
    sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // 前面：z = max.z()
    sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // 右面：x = max.x()
    sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // 后面：z = min.z()
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // 左面：x = min.x()
    sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // 顶面：y = max.y()
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // 底面：y = min.y()

    return sides;
}

#endif
