
#pragma once

//为光线可能击中的任何物体创建一个“抽象类”
#include "ray.h"

class hit_record {
public:
    point3 p; //交点
    vec3 normal; //法线
    double t; //光线的t
    bool front_face; //光线打在内表面还是外表面 true为外, false为内

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal; //这里让法线始终与光线方向相反
    }
};

class hittable {
public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const = 0;
};