
#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "color.h"
#include "material.h"

// camera 负责把三维世界渲染成二维图像。
// 它根据相机参数生成穿过每个像素的光线，并递归追踪光线颜色。
class camera {
public:
    double aspect_ratio = 1.0;  // 图像宽高比（宽 / 高），决定最终图像高度。
    int    image_width  = 100;  // 输出图像宽度（像素），也是渲染分辨率的主要控制项。
    int    samples_per_pixel = 10; // 每个像素的随机采样数量，用来抗锯齿和降低蒙特卡洛噪点。
    int    max_depth = 10; // 最大递归反弹次数，防止光线在镜面或玻璃间无限追踪。

    double vfov = 90; // 垂直视场角，决定画面能看到多大的上下范围
    point3 lookfrom = point3(0,0,0); // 相机位置，也就是光线默认发出的点
    point3 lookat = point3(0,0,-1); // 相机看向的目标点
    vec3 vup = vec3(0,1,0); // 相机的“上方”参考方向，用来确定画面不会倾斜


    double defocus_angle = 0; // 散焦角度，角度越大，光圈越大，景深越浅
    double focus_dist = 10; // 对焦距离，从相机到焦平面的距离

    // 渲染入口：初始化相机内部参数，然后逐像素计算颜色并写出 PPM 文本。
    // world 是被拍摄的场景，所有光线都会向它查询最近命中。
    void render(const hittable& world) {
        initialize();

        // PPM P3 文件头：告诉查看器图像格式、宽高和颜色最大值。
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        // 逐行逐列遍历像素；每个像素会发射多条略有偏移的采样光线。
        for (int j = 0; j < image_height; j++) {
            std::clog << "\r剩余扫描线: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                // 累加该像素的所有采样颜色，最后统一乘以平均系数。
                color pixel_color(0,0,0);
                for(int sample = 0; sample < samples_per_pixel; sample++){
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth,world);
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }

        std::clog << "\r完成。                 \n";
    }

private:
    int    image_height;   // 输出图像高度（像素），由宽度和宽高比计算得到。
    point3 center;         // 相机中心，也是无景深时所有主光线的起点。
    double pixel_samples_scale; // 采样平均系数，等于 1 / samples_per_pixel。
    point3 pixel00_loc;    // 左上角第一个像素中心位置，是遍历像素时的起算点。
    vec3   pixel_delta_u;  // 向右一个像素的位移，用来定位第 i 列像素。
    vec3   pixel_delta_v;  // 向下一个像素的位移，用来定位第 j 行像素。
    vec3   u, v, w;         // 相机坐标系基向量：u 向右，v 向上，w 从目标点指回相机
    vec3   defocus_disk_u;  // 散焦圆盘在相机右方向上的半径向量
    vec3   defocus_disk_v;  // 散焦圆盘在相机上方向上的半径向量

    // 根据公开的相机参数计算渲染所需的内部缓存。
    // 这些缓存让 get_ray() 在每个像素上快速生成光线，不必重复计算视口和坐标系。
    void initialize() {
        // 根据宽高比计算高度，并保证至少为 1。
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        // 相机中心使用 lookfrom，使光线从相机位置出发。
        center = lookfrom;

        // 根据垂直视场角和对焦距离计算视口尺寸。
        auto theta = degrees_to_radians(vfov); // 垂直视场角（弧度制）
        auto h = std::tan(theta/2); // 半视口高度和焦距的比例。
        auto viewport_height = 2 * h * focus_dist;

        // 视口宽度与实际像素宽高比一致，避免图像被拉伸。
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // 计算相机基向量，把世界坐标转换成相机自己的右、上、后方向。
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup,w));
        v = cross(w, u);

        // 计算视口水平和竖直方向边向量。
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        // 计算像素间在水平和竖直方向的步进向量。
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // 计算视口左上角以及首像素中心位置。
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // 根据散焦角度和对焦距离，计算模拟光圈的圆盘半径。
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    // 为像素 (i,j) 生成一条采样光线。
    // 作用是把二维像素坐标转换为三维世界中的一条射线，供 ray_color() 追踪。
    ray get_ray(int i, int j) const {
        // 构造一条相机光线，目标是像素 i,j 附近的随机采样点。
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                            + ((i + offset.x())) * pixel_delta_u
                            + ((j + offset.y())) * pixel_delta_v; // 计算采样点在世界坐标中的位置。

        // 没有景深时从相机中心发射；开启景深时从光圈圆盘内随机点发射。
        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    // 在像素中心周围的单位方格内生成随机偏移。
    // 它让多次采样覆盖像素面积，从而平滑边缘锯齿和随机噪声。
    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    // 在相机散焦圆盘内生成随机起点。
    // 多条光线从不同光圈位置穿过同一像素，可以模拟真实镜头的景深效果。
    point3 defocus_disk_sample() const {
        // 返回相机散焦圆盘内的随机点，用来模拟真实镜头的有限光圈。
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    // 递归计算一条光线看到的颜色。
    // 命中物体时交给材质生成散射光线，未命中时返回天空背景。
    color ray_color(const ray& r, int depth, const hittable& world) const {
        hit_record rec;
        // 递归深度耗尽时返回黑色，表示这条光线不再贡献能量。
        if(depth <= 0)
            return color(0,0,0);

        // 用 0.001 作为最小 t，避免散射光线从表面出发后立刻再次击中同一表面。
        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            // 材质决定散射方向和颜色衰减；递归结果乘以衰减得到最终反弹贡献。
            if(rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth-1, world);
            return color(0,0,0);
        }

        // 未命中时返回天空渐变背景。
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }
};

#endif
