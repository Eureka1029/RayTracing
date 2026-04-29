#ifndef SPHERE_H
#define SPHERE_H


#include "hittable.h"
#include "vec3.h"


class sphere : public hittable {
public:
    sphere(const point3& center, double radius, std::shared_ptr<material> mat) 
    : center(center), radius(std::fmax(0, radius)) {
        // 需要初始化材质指针mat
    };

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
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
        if (!ray_t.surrounds(root)) {  //不在区间内
            root = (h + sqrtd) / a; //再看+号根
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t); //光线与球的交点
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;
        
        return true;
    }
private:
    point3 center; //球心
    double radius; //半径
    std::shared_ptr<material> mat; // 材质指针
    
};

#endif