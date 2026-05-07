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

//介电材质
class dielectric : public material {
public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index; // 从空气射向物体和物体射向空气,求出折射率之比.

        vec3 unit_direction = unit_vector(r_in.direction()); // 入射光线归一化
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0); //求出cos
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1; // 决定了能不能折射
        vec3 direction;

        if(cannot_refract || reflectance(cos_theta, ri) > random_double()){
            direction = reflect(unit_direction, rec.normal); //反射
        }else{
            direction = refract(unit_direction,rec.normal,ri); //折射
        }


        scattered = ray(rec.p, direction); 
        return true;
    }
public:
    double refraction_index;

    static double reflectance(double cosine, double refraction_index) { 
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1-cosine),5);
    }
};

#endif
