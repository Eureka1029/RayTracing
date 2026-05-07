#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"


// 程序入口：负责搭建场景、配置相机，并启动渲染。
// 真正的光线求交、材质散射和颜色输出分别由各个类完成，main 只组织整体流程。
int main() {
    // world 是场景容器；所有球体都会加入这里，相机渲染时只查询这个统一对象。
    hittable_list world;


    // 创建大地面：用一个半径很大的球体模拟平坦地面，材质为灰色漫反射。
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    // 随机生成一片小球阵列，用不同材质测试漫反射、金属和玻璃的渲染效果。
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            // choose_mat 决定小球材质类型；center 给格子位置增加随机偏移，让场景更自然。
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            // 避开右侧大球附近的位置，防止随机小球和主角球重叠。
            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // 漫反射小球：随机反照率让地面上出现大量柔和、哑光的彩色球。
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // 金属小球：随机颜色和粗糙度产生从镜面到磨砂金属的变化。
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // 玻璃小球：折射率 1.5 模拟普通玻璃，负责提供透明折射效果。
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    // 中间大球：玻璃材质，用来展示反射、折射和全内反射。
    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    // 左侧大球：棕色漫反射材质，用来和金属、玻璃材质形成对比。
    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    // 右侧大球：低粗糙度金属材质，用来展示清晰的镜面反射。
    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    // camera 负责根据下面的参数发射光线，并把渲染结果写成 PPM 图片。
    camera cam;

    // 输出参数：宽高比、图像宽度、每像素采样数和递归反弹上限。
    // 更高采样数会减少噪点，更高递归深度能表现更多次反射/折射。
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 50;

    // 相机取景参数：位置、观察目标和上方向共同决定画面构图。
    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    // 景深参数：散焦角越大虚化越明显，对焦距离决定清晰平面的位置。
    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    // 启动渲染；结果写到标准输出，进度写到标准错误。
    cam.render(world);
}
