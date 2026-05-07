#ifndef COLOR_H
#define COLOR_H
#include "vec3.h"
#include "interval.h"

// color 复用 vec3 的三个分量，含义从几何坐标变为 RGB 颜色。
// 这样颜色可以直接使用向量的加法、乘法和缩放运算，便于累加采样结果。
using color = vec3;

// 将线性空间的颜色分量转换到 gamma=2 的显示空间。
// 作用是让输出图像在普通显示器上看起来更接近人眼感知，避免画面过暗。
inline double linear_to_gamma(double linear_component){
    if(linear_component > 0)
        return std::sqrt(linear_component);
    
    return 0;
}

// 把一个像素的浮点 RGB 颜色写成 PPM 格式中的 0-255 整数。
// render() 每算完一个像素都会调用它，最终标准输出就是整张图片文件内容。
inline void write_color(std::ostream& out, const color& pixel_color) {
    // 取出三个颜色通道；它们通常是多次采样平均后的线性颜色值。
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // 做 gamma 校正；这是写入文件前的显示适配步骤。
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // 将 [0,1] 的颜色限制并映射到 [0,255]，0.999 可避免 1.0 被转成 256。
    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));


    // 以文本形式输出一个像素的 RGB 值；PPM 的 P3 格式要求每个通道用整数表示。
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif
