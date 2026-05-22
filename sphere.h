#ifndef SPHERE_H
#define SPHERE_H


#include "hittable.h"
#include "vec3.h"


// sphere 表示一个球体几何对象，同时实现 hittable 接口。
// 它负责计算光线与球是否相交，并把交点信息交给材质和相机继续处理。
class sphere : public hittable {
public:
    // 创建球体；半径被限制为非负数，材质指针决定球体表面的散射行为。
    sphere(const point3& center, double radius, std::shared_ptr<material> mat) 
        : center(center), radius(std::fmax(0, radius)), mat(mat) 
    {
        auto rvec = vec3(radius,radius,radius); 
        bbox = aabb(center - rvec, center + rvec); //创建包围盒
    };

    // 判断光线是否在指定 t 区间内击中球体。
    // 作用是把几何求交结果转换成 hit_record，供渲染递归计算颜色。
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // oc 是从光线起点指向球心的向量，用于把球面方程代入光线方程。
        vec3 oc = center - r.origin(); 

        // a、h、c 是二次方程的系数；这里使用 h 简化原本的 b/2 写法。
        auto a = dot(r.direction(), r.direction());
        auto h = dot(r.direction(), oc);
        auto c = dot(oc, oc) - radius*radius;

        // 判别式小于 0 代表光线完全错过球体。
        auto discriminant = h*h - a*c;
    
        if(discriminant < 0.0)
            return false;
        
        auto sqrtd = std::sqrt(discriminant);

        // 求两个可能的命中距离 t，优先选择更近的根。
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            // 较近根无效时再尝试较远根，例如光线从球内向外射出。
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        // 写入命中结果：距离、交点、朝向修正后的法线和材质。
        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;
        
        return true;
    }

    aabb bounding_box() const override { return bbox; }
private:
    point3 center; // 球心位置：决定球体放在世界坐标中的哪里。
    double radius; // 球体半径：决定球体大小，也参与法线计算。
    std::shared_ptr<material> mat; // 材质指针：决定命中这个球后光线如何散射。
    aabb bbox;

    // 计算球体表面上一点的纹理坐标 (u, v)，用于纹理映射。
    // p: 单位球面上的点（球心在原点、半径为 1）。
    // u: 返回值 [0,1]，表示绕 Y 轴的角度，从 X=-1 开始。
    // v: 返回值 [0,1]，表示从 Y=-1 到 Y=+1 的角度。
    //     <1 0 0> 对应 <0.50 0.50>     <-1  0  0> 对应 <0.00 0.50>
    //     <0 1 0> 对应 <0.50 1.00>     < 0 -1  0> 对应 <0.50 0.00>
    //     <0 0 1> 对应 <0.25 0.50>     < 0  0 -1> 对应 <0.75 0.50>
    static void get_sphere_uv(const point3& p, double& u, double& v) {
        auto theta = std::acos(-p.y());
        auto phi = std::atan2(-p.z(), p.x()) + pi;

        u = phi / (2*pi);
        v = theta / pi;
    }
};

#endif
