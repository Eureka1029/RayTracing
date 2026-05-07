#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>
#include "rtweekend.h"

// vec3 是三维向量类，用来表达位置、方向、颜色等三分量数据。
// 光线追踪中几乎所有几何计算都依赖它，例如点乘、叉乘、归一化、反射和折射。
class vec3{
    
public:
    double e[3]; // 三个分量数组：既可表示 x/y/z，也可表示 r/g/b。

    // 默认构造为零向量；适合作为颜色累加、方向占位或初始化值。
    vec3() : e{0,0,0} {}

    // 使用三个分量构造向量；调用者可以明确创建点、方向或颜色。
    vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

    // 访问 x/y/z 分量；让代码在几何语义下更清晰。
    double x() const {return e[0]; }
    double y() const {return e[1]; }
    double z() const {return e[2]; }

    // 取反向量；常用于翻转法线或得到反方向光线。
    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }

    // 只读下标访问；方便按数组形式读取分量，例如散焦圆盘采样。
    double operator[](int i) const { return e[i]; }

    // 可写下标访问；允许直接修改某个分量。
    double& operator[](int i) { return e[i]; }

    // 向量累加；渲染时用于把多次采样颜色加到同一个像素上。
    vec3& operator+=(const vec3& other){
        e[0] += other.e[0];
        e[1] += other.e[1];
        e[2] += other.e[2];
        return *this;
    }

    // 向量按标量缩放；用于改变方向长度或颜色强度。
    vec3& operator*=(double t){
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    // 向量除以标量；本质是乘以倒数，常用于平均颜色和归一化。
    vec3 operator/=(double t){
        return *this *= 1/t;
    }

    // 返回向量长度；用于距离判断、归一化和物体位置筛选。
    double length() const {
        return std::sqrt(length_squared());
    }

    // 返回长度平方；避免不必要的开方，常用于比较距离或判断是否在单位球内。
    double length_squared() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }

    // 判断向量是否接近零；漫反射采样中用它避免生成退化方向。
    bool near_zero() const {
        auto s = 1e-8;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }

    // 生成 [0,1) 范围内的随机向量；常用于随机颜色。
    static vec3 random() {
        return vec3(random_double(), random_double(), random_double());
    }

    // 生成 [min,max) 范围内的随机向量；常用于随机方向或可控颜色范围。
    static vec3 random(double min, double max) {
        return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
    }
};

// point3 是 vec3 的语义别名：同样是三个数，但表示空间中的点。
// 这个别名让代码能区分“位置”和“方向”，读起来更接近光线追踪概念。
using point3 = vec3;

// 输出向量三个分量；调试几何值或写出颜色文本时会用到。
inline std::ostream& operator<<(std::ostream& out, const vec3& v){
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

// 向量加法；用于点偏移、颜色累加、方向组合等。
inline vec3 operator+(const vec3& u, const vec3& v){
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

// 向量减法；用于得到两点之间的方向或距离向量。
inline vec3 operator-(const vec3& u, const vec3& v){
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

// 分量相乘；颜色中表示逐通道衰减，几何中较少使用。
inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

// 标量乘向量；用于缩放方向长度或颜色亮度。
inline vec3 operator*(double t, const vec3& v) {
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

// 向量乘标量；提供更自然的书写顺序，内部复用标量乘向量。
inline vec3 operator*(const vec3& v, double t) {
    return t * v;
}

// 向量除以标量；用于平均采样结果和单位化向量。
inline vec3 operator/(const vec3& v, double t) {
    return (1/t) * v;
}

// 点乘：衡量两个方向的夹角关系，也用于投影和求交方程。
inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
         + u.e[1] * v.e[1]
         + u.e[2] * v.e[2];
}

// 叉乘：生成同时垂直于两个输入向量的新向量，相机坐标系构建依赖它。
inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

// 归一化：把向量变成长度为 1 的方向向量，便于只关心方向而不关心长度。
inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

// 在单位球面上生成随机方向。
// 作用是为漫反射和金属模糊反射提供随机散射方向，形成柔和的全局光照效果。
inline vec3 random_unit_vector() {
    while(true){
        auto p = vec3::random(-1,1);
        auto lensq = p.length_squared(); 
        if(1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}

// 在 xy 平面的单位圆盘内生成随机点。
// 相机用它在光圈圆盘上随机选择发光点，从而模拟景深和背景虚化。
inline vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

// 在给定法线所在的半球内生成随机方向。
// 作用是保证散射方向朝向物体外侧，避免光线随机射入表面内部。
inline vec3 random_on_hemisphere(const vec3& normal){
    vec3 on_unit_sphere = random_unit_vector();
    if(dot(on_unit_sphere,normal) > 0.0){
        return on_unit_sphere;
    }else{
        return -on_unit_sphere;
    }
}

// 根据法线 n 计算入射方向 v 的镜面反射方向。
// 金属材质使用它产生高光和镜面反射效果。
inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2*dot(v,n)*n;
}


// 根据斯涅尔定律计算折射方向。
// uv 是单位入射方向，n 是表面法线，etai_over_etat 是入射介质与出射介质的折射率之比。
// 介电材质使用它模拟玻璃、水晶等透明材料中的光线弯折。
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

#endif
