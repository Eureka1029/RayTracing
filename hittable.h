
#pragma once

//为光线可能击中的任何物体创建一个“抽象类”
#include "ray.h"

class hit_record {
public:
    point3 p; //交点
    vec3 normal; //法线
    double t; //光线的t
};

class hittable {
public:
    virtual ~hittable() = default;
    
    virtual bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const = 0;
};