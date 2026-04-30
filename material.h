#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "color.h"

class material {
    public:
        virtual ~material() = default;

        virtual bool scatter( 
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const { // 入射光 hit记录 散射颜色  散射光
            return false;
        }
        
};

class lambertian : public material { //朗伯反射模型
public:
    lambertian(const color& albedo) : albedo(albedo) {};

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        auto scatter_direction = rec.normal + random_unit_vector(); // 折射光方向,如果随机生成的向量正好和法向量相反则会产生严重的问题

        if(scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction); //折射光
        attenuation = albedo;
        return true;
    }
private:
    color albedo;

};

class metal : public material {
public:
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}
bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal); // 镜面反射方向
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector()); //给镜面反射添加一些扰动
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0); //判断散射方向是否朝外
    }
private:
    color albedo;
    double fuzz;
};

#endif
