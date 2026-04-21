#pragma once

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
public:
    sphere(const point3& center, double radius) : center(center), radius(std::fmax(0, radius)) {};

    bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const override {
        vec3 oc = center - r.origin(); 
        auto a = dot(r.direction(), r.direction());
        auto h = dot(r.direction(), oc); // b = -2h化简后结果
        auto c = dot(oc, oc) - radius*radius;

        auto discriminant = h*h - a*c; //判别式
    
        if(discriminant < 0.0)
            return false;
        
        auto sqrtd = std::sqrt(discriminant);

        //求根公式的两个t,找出离摄像头最近的t
        auto root = (h - sqrtd) / a; //先看-号根,越小越近
        if (root <= ray_tmin || ray_tmax <= root) {  //不在区间内
            root = (h + sqrtd) / a; //再看+号根
            if (root <= ray_tmin || ray_tmax <= root)
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t); //光线与球的交点
        rec.normal = (rec.p - center) / radius; //归一化的法线

        return true;
    }
private:
    point3 center; //球心
    double radius; //半径
    
};