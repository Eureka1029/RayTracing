#ifndef TEXTURE_H
#define TEXTURE_H

#include"color.h"
#include"vec3.h"

// texture 是所有纹理的抽象基类。
// 给定 uv 坐标和命中点，返回该点的颜色值，支持纯色、棋盘格、图像纹理等。
class texture {
  public:
    virtual ~texture() = default;

    // 纹理采样：根据 uv 坐标和命中点 p 返回颜色。
    virtual color value(double u, double v, const point3& p) const = 0;
};

// solid_color 是纯色纹理实现：无论 uv 和命中点如何，始终返回同一颜色。
class solid_color : public texture {
  public:
    solid_color(const color& albedo) : albedo(albedo) {}

    solid_color(double red, double green, double blue) : solid_color(color(red,green,blue)) {}

    color value(double u, double v, const point3& p) const override {
        return albedo;
    }

  private:
    color albedo; // 纯色纹理的颜色值。
};

// checker_texture 是棋盘格纹理实现。
// 根据命中点在空间中所在的立方体网格，交替返回两种纹理的颜色。
// 两个纹理通过 shared_ptr 共享持有，允许多个物体复用同一纹理实例。
class checker_texture : public texture {
  public:
    // 构造函数：传入缩放系数和两个子纹理。
    // inv_scale 的倒数决定棋盘格的大小，scale 越大格子越小。
    checker_texture(double scale, std::shared_ptr<texture> even, std::shared_ptr<texture> odd)
      : inv_scale(1.0 / scale), even(even), odd(odd) {}

    // 便捷构造函数：传入两种颜色，内部自动创建 solid_color 纹理。
    checker_texture(double scale, const color& c1, const color& c2)
      : checker_texture(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2)) {}

    color value(double u, double v, const point3& p) const override {
        auto xInteger = int(std::floor(inv_scale * p.x()));
        auto yInteger = int(std::floor(inv_scale * p.y()));
        auto zInteger = int(std::floor(inv_scale * p.z()));

        // 将 x+y+z 的整数部分之和的奇偶性作为棋盘格判定依据。
        bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

  private:
    double inv_scale; // 缩放系数的倒数：值越大格子越小，用于将世界坐标映射到棋盘格网格。
    std::shared_ptr<texture> even; // 偶数格纹理指针：多个物体可共享同一纹理实例。
    std::shared_ptr<texture> odd;  // 奇数格纹理指针：多个物体可共享同一纹理实例。
};

#endif
