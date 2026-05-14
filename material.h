#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "color.h"


// material 是材质抽象基类，定义光线命中表面后如何散射。
// 不同材质通过实现 scatter() 决定新光线方向和颜色衰减，从而形成漫反射、金属和玻璃效果。
class material {
    public:
        // 虚析构函数：保证通过基类指针释放派生材质时行为正确。
        virtual ~material() = default;

        // scatter 计算一次命中后的散射结果。
        // r_in 是入射光，rec 是命中信息，attenuation 是颜色衰减，scattered 是下一条光线。
        // 返回 false 表示光线被吸收，递归追踪到此结束。
        virtual bool scatter( 
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const {
            return false;
        }
        
};

// lambertian 表示理想漫反射材质。
// 它把光线随机散射到法线附近的方向，用来模拟粗糙、无镜面高光的表面。
class lambertian : public material {
public:
    // albedo 是反照率，也就是材质保留下来的颜色比例。
    lambertian(const color& albedo) : albedo(albedo) {};

    // 漫反射散射：在法线附近随机取一个方向，并让光线从命中点继续传播。
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        // 法线加随机单位向量可得到偏向外半球的随机方向，形成柔和的漫反射。
        auto scatter_direction = rec.normal + random_unit_vector();

        // 极小概率下随机方向与法线抵消，退化成零向量；这里回退到法线方向。
        if(scatter_direction.near_zero())
            scatter_direction = rec.normal;

        // 从命中点发出散射光线；attenuation 决定这次反弹保留多少颜色。
        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }
private:
    // 材质固有颜色；每次反弹都会按这个颜色衰减光能。
    color albedo;

};

// metal 表示金属材质。
// 它以镜面反射为基础，并通过 fuzz 增加随机扰动来模拟不同粗糙度的金属表面。
class metal : public material {
public:
    // fuzz 被限制在 [0,1] 内；越小越像镜子，越大反射越模糊。
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    // 金属散射：先计算理想反射，再叠加随机扰动形成粗糙反射。
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attenuation = albedo;
        // 如果扰动后的方向射入表面内部，则视为无效散射，避免错误的内部反弹。
        return (dot(scattered.direction(), rec.normal) > 0);
    }
private:
    // 金属颜色；常用于模拟铜、银、金等不同反射色。
    color albedo;

    // 表面粗糙度；控制反射方向的随机偏移幅度。
    double fuzz;
};

// dielectric 表示介电材质，例如玻璃或水。
// 它允许光线反射或折射，并用概率近似菲涅尔效应。
class dielectric : public material {
public:
    // refraction_index 是材质折射率；玻璃通常约为 1.5。
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    // 介电散射：根据入射角、折射率和反射概率决定光线反射还是折射。
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        // 透明材质不吸收颜色，所以衰减保持白色。
        attenuation = color(1.0, 1.0, 1.0);

        // 根据光线是进入还是离开材质，选择正确的折射率之比。
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        // 用单位入射方向计算入射角，方便判断是否发生全反射。
        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        // 当折射公式无解时必须反射，这就是全内反射。
        bool cannot_refract = ri * sin_theta > 1;
        vec3 direction;

        // Schlick 近似给出随角度变化的反射概率，使玻璃边缘更容易反光。
        if(cannot_refract || reflectance(cos_theta, ri) > random_double()){
            direction = reflect(unit_direction, rec.normal);
        }else{
            direction = refract(unit_direction,rec.normal,ri);
        }


        // 反射或折射后的光线继续参与递归追踪。
        scattered = ray(rec.p, direction); 
        return true;
    }
public:
    // 折射率：决定光线穿过材质时弯折程度。
    double refraction_index;

    // Schlick 反射率近似；用低成本公式模拟菲涅尔反射强度变化。
    static double reflectance(double cosine, double refraction_index) { 
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1-cosine),5);
    }
};

#endif
