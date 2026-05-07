
#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "color.h"
#include "material.h"

class camera {
public:
    double aspect_ratio = 1.0;  // 图像宽高比（宽 / 高）
    int    image_width  = 100;  // 输出图像宽度（像素）
    int    samples_per_pixel = 10; //一个像素的采样点数量
    int    max_depth = 10; // 最大光线反射次数

    double vfov = 90; // 垂直视场角，决定画面能看到多大的上下范围
    point3 lookfrom = point3(0,0,0); // 相机位置，也就是光线默认发出的点
    point3 lookat = point3(0,0,-1); // 相机看向的目标点
    vec3 vup = vec3(0,1,0); // 相机的“上方”参考方向，用来确定画面不会倾斜


    double defocus_angle = 0; // 散焦角度，角度越大，光圈越大，景深越浅
    double focus_dist = 10; // 对焦距离，从相机到焦平面的距离

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        // 逐行逐列遍历像素并写出颜色。
        for (int j = 0; j < image_height; j++) {
            std::clog << "\r剩余扫描线: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
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
    int    image_height;   // 输出图像高度（像素）
    point3 center;         // 相机中心
    double pixel_samples_scale; //所有像素采样点颜色规模因子
    point3 pixel00_loc;    // 左上角第一个像素中心位置
    vec3   pixel_delta_u;  // 向右一个像素的位移
    vec3   pixel_delta_v;  // 向下一个像素的位移
    vec3   u, v, w;         // 相机坐标系基向量：u 向右，v 向上，w 从目标点指回相机
    vec3   defocus_disk_u;  // 散焦圆盘在相机右方向上的半径向量
    vec3   defocus_disk_v;  // 散焦圆盘在相机上方向上的半径向量

    void initialize() {
        // 根据宽高比计算高度，并保证至少为 1。
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        // 相机中心使用 lookfrom，使光线从相机位置出发。
        center = lookfrom;

        // 根据垂直视场角和对焦距离计算视口尺寸。
        auto theta = degrees_to_radians(vfov); // 垂直视场角（弧度制）
        auto h = std::tan(theta/2); 
        auto viewport_height = 2 * h * focus_dist;

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

    ray get_ray(int i, int j) const {
        // 构造一条相机光线，目标是像素 i,j 附近的随机采样点。
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                            + ((i + offset.x())) * pixel_delta_u
                            + ((j + offset.y())) * pixel_delta_v; //计算出采样点坐标

        // 没有景深时从相机中心发射；开启景深时从光圈圆盘内随机点发射。
        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const { //作用在步长上的偏移系数
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // 返回相机散焦圆盘内的随机点，用来模拟真实镜头的有限光圈。
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        hit_record rec;
        if(depth <= 0)
            return color(0,0,0);

        // 命中物体时根据法线返回可视化颜色。
        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
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
