#ifndef QUAD_H
#define QUAD_H

#include "hittable.h"

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

#endif
