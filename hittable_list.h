#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

#include <memory>
#include <vector>

using std::make_shared;
using std::shared_ptr;

// hittable_list 把多个 hittable 组合成一个“世界”对象。
// 相机渲染时只需要询问这个列表是否被击中，列表内部负责找出最近的物体。
class hittable_list : public hittable {
public:
    // 保存场景中所有可命中的物体；使用 shared_ptr 方便多个地方共享对象生命周期。
    std::vector<shared_ptr<hittable>> objects;

    // 创建空场景；main.cpp 会用它逐步 add 球体。
    hittable_list() {}

    // 用单个物体创建场景；适合测试或快速构造只有一个对象的世界。
    hittable_list(shared_ptr<hittable> object) { add(object); }

    // 清空场景对象；重新生成世界时可复用同一个列表。
    void clear() { objects.clear(); }

    // 添加一个可命中物体；场景构建阶段通过它把球体放进世界。
    void add(shared_ptr<hittable> object) {
        objects.push_back(object);
    }

    // 遍历所有物体，寻找 ray_t 范围内离光线起点最近的命中点。
    // 作用是让前面的物体遮挡后面的物体，得到正确的可见表面。
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for(const auto& object : objects){
            // 每找到一个更近命中，就缩小最大 t，后续物体只能在它之前竞争。
            if(object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)){
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        
        return hit_anything;
    }

};

#endif
