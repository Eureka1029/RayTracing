
#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "color.h"


class camera {
public:
    double aspect_ratio = 1.0;  // 图像宽高比（宽 / 高）
    int    image_width  = 100;  // 输出图像宽度（像素）
    int    samples_per_pixel = 10; //一个像素的采样点数量
    int    max_depth = 10; // 最大光线反射次数
    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        // 逐行逐列遍历像素并写出颜色。
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0,0,0);
                for(int sample = 0; sample < samples_per_pixel; sample++){
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth,world);
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

private:
    int    image_height;   // 输出图像高度（像素）
    point3 center;         // 相机中心
    double pixel_samples_scale; //所有像素采样点颜色规模因子
    point3 pixel00_loc;    // 左上角第一个像素中心位置
    vec3   pixel_delta_u;  // 向右一个像素的位移
    vec3   pixel_delta_v;  // 向下一个像素的位移

    void initialize() {
        // 根据宽高比计算高度，并保证至少为 1。
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        // 相机位于原点。
        center = point3(0, 0, 0);

        // 计算视口尺寸。
        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // 计算视口水平和竖直方向边向量。
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        // 计算像素间在水平和竖直方向的步进向量。
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // 计算视口左上角以及首像素中心位置。
        auto viewport_upper_left =
            center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    }

    ray get_ray(int i, int j) const {
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                            + ((i + offset.x())) * pixel_delta_u
                            + ((j + offset.y())) * pixel_delta_v; //计算出采样点坐标

        auto ray_origin = center;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const { //作用在步长上的偏移系数
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        hit_record rec;
        if(depth <= 0)
            return color(0,0,0);

        // 命中物体时根据法线返回可视化颜色。
        if (world.hit(r, interval(0.001, infinity), rec)) {
            vec3 direction = rec.normal + random_unit_vector(); //相当于对一个表面向量增加一个随机扰动
            return 0.5 * ray_color(ray(rec.p, direction),depth-1, world);
        }

        // 未命中时返回天空渐变背景。
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }
};

#endif
