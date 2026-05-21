#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "interval.h"
#include "aabb.h"

// 前向声明 material，避免在这里只为指针成员引入完整材质定义，减少头文件耦合。
class material;

// hit_record 保存一次光线命中的全部结果。
// 材质散射、阴影判断和颜色递归都依赖这里记录的交点、法线、材质和距离。
class hit_record {
public:
    point3 p; // 交点坐标：材质会从这里发出下一条散射光线。
    vec3 normal; // 命中点法线：用于计算反射、折射和漫反射随机方向。
    std::shared_ptr<material> mat; // 命中物体的材质：决定光线如何散射和衰减。
    double t; // 光线参数 t：表示交点沿光线方向的距离，用来比较最近命中。
    double u; // 命中点纹理坐标 u：用于纹理贴图采样，范围 [0,1]。
    double v; // 命中点纹理坐标 v：与 u 一起构成 2D 纹理坐标，范围 [0,1]。
    bool front_face; // 是否打在外表面：true 为外侧，false 为内侧，折射材质需要它判断折射率方向。

    // 根据入射光线方向修正法线方向。
    // 作用是保证 rec.normal 总是和入射光线相反，简化后续材质计算。
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// hittable 是所有可被光线击中物体的抽象接口。
// 相机只依赖这个接口渲染世界，因此球体、列表或未来的其他几何体都能统一参与求交。
class hittable {
public:
    // 虚析构函数：允许通过基类指针安全释放派生类对象。
    virtual ~hittable() = default;

    // 判断光线 r 在 ray_t 区间内是否击中物体，并把最近的有效命中写入 rec。
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

    virtual aabb bounding_box() const = 0; //aabb包围盒
};

#endif
