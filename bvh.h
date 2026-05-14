#ifndef BVH_H
#define BVH_H

#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"
#include <algorithm>
#include "rtweekend.h"

// BVH 节点本身也是一个 hittable。
// 它不是直接表示某个具体物体，而是把一组物体分成左右两部分，
// 并用一个大的包围盒 bbox 把这两部分整体包起来。
// 光线求交时可以先测试 bbox，如果光线连大包围盒都没碰到，
// 就不用继续检查这个节点下面的所有物体，从而加速渲染。
class bvh_node : public hittable {
  public:
    // 从一个 hittable_list 构建整棵 BVH。
    // 这里把整个 list 的 objects 数组交给另一个构造函数处理，
    // start 为 0，end 为 objects.size()，表示使用全部物体。
    bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {
        // 这里有一个 C++ 的细节。这个构造函数没有传入区间下标，它会隐式拷贝一份
        // hittable_list，而后续构建 BVH 时会修改这份拷贝。这个拷贝出来的 list 生命周期
        // 只持续到构造函数结束。这样没问题，因为我们真正需要保留下来的是最终构建出的
        // 包围体层次结构。
    }

    // 从 objects[start, end) 这一段物体构建一个 BVH 节点。
    // 这个函数通常会：
    // 1. 选择一个坐标轴；
    // 2. 按物体包围盒在该轴上的位置排序；
    // 3. 把物体分成左右两半；
    // 4. 递归构建 left 和 right；
    // 5. 用左右子节点的包围盒合并出当前节点的 bbox。
    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        bbox = aabb::empty; //当前节点初始化为空盒子
        for (size_t object_index=start; object_index < end; object_index++)
            bbox = aabb(bbox, objects[object_index]->bounding_box()); //和每一个物体的包围盒合并

        int axis = bbox.longest_axis(); //这个包围盒在哪个方向上最长 0, 1, 2


        auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare; //将随机数转换成x,y,z轴

        size_t object_span = end - start; //计算区间里有多少个物体

        if (object_span == 1) {
            left = right = objects[start]; //只有一个物体 
            //为什么会指向同一个? 原因是后面的代码会检验光线是否射中左右指针,这样写不需要特判
        } else if (object_span == 2) { //只有两个物体,左右指针各指一个
            left = objects[start]; 
            right = objects[start+1];
        } else { 
            //多个物体
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator); //按照选定的坐标轴排序

            auto mid = start + object_span/2;
            //递归对半切
            left = make_shared<bvh_node>(objects, start, mid); 
            right = make_shared<bvh_node>(objects, mid, end);
        }

        bbox = aabb(left->bounding_box(), right->bounding_box()); //合并成包围盒作为头节点
    }

    // 判断光线是否击中当前 BVH 节点里的任意物体。
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // 先用当前节点的大包围盒做快速测试。
        // 如果光线没有进入这个包围盒，那么它不可能打中里面的任何物体。
        if (!bbox.hit(r, ray_t))
            return false;

        // 光线打中了大包围盒，才继续检查左右子节点。
        bool hit_left = left->hit(r, ray_t, rec);

        // 如果左子节点已经命中，rec.t 会记录当前最近的命中距离。
        // 右子节点只需要寻找比 rec.t 更近的交点；更远的交点即使命中也不会被采用。
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        // 只要左边或右边任意一个命中，就说明当前 BVH 节点被命中。
        return hit_left || hit_right;
    }

    // 返回当前节点的包围盒。
    // 对叶子节点来说，它包住具体物体；
    // 对内部节点来说，它包住 left 和 right 两个子节点。
    aabb bounding_box() const override { return bbox; }

  private:
    // 左右子节点可以是具体物体，比如 sphere；
    // 也可以是另一个 bvh_node，因此类型统一写成 shared_ptr<hittable>。
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;

    //按照某轴进行排序的比较器
    static bool box_compare(
        const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index
    ) {
        auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
        auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 2);
    }
    // 当前节点的轴对齐包围盒，包住 left 和 right 代表的所有物体。
    aabb bbox;
};

#endif
