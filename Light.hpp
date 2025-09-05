#ifndef LIGHT_H
#define LIGHT_H
#include "Vec3.hpp"
#include <cmath>
#include <random>
enum class LightType {
    Point,
    Triangle
};
// 点光源
template<typename T = float>
class Light {
    public:
    virtual ~Light() = default;
    virtual LightType getType() const = 0;
};
template<typename T = float>
class PointLight : public Light<T> {
public:
    Vec3<T> position, color;
    // 点光源  强度 Intensity I	（W/sr）	没有面积，随距离平方衰减
    PointLight(const Vec3<T>& pos, const Vec3<T>& col) : position(pos), color(col) {}
    LightType getType() const override { return LightType::Point; }
};
template<typename T = float>
class TriangleLight : public Light<T> {
public:
    Vec3<T> A, B, C, color, normal;
    T area;
    // 面光源	辐射亮度 Radiance 𝐿（L	W/(sr·m²)）
    TriangleLight(const Vec3<T>& __A, const Vec3<T>& __B, const Vec3<T>& __C,
                  const Vec3<T>& __color)
        : A(__A), B(__B), C(__C), color(__color) { compute(); }
    void compute() {
        // 预计算法线与面积（高频使用，避免每次采样都算）
        const Vec3<T> n = (B - A).cross(C - A);
        const T doubleArea = n.length();
        area = T(0.5) * doubleArea;
        normal = (doubleArea > T(0)) ? (n / doubleArea) : Vec3<T>(0, 0, 0);
    }
    LightType getType() const override { return LightType::Triangle; }
    template<typename URNG>
    inline Vec3<T> samplePoint(URNG& rng) const {
        std::uniform_real_distribution<T> uni(0, 1);
        const T r1 = std::sqrt(uni(rng));
        const T u = T(1) - r1;
        const T v = uni(rng) * r1;
        return A * u + B * v + C * (T(1) - u - v);
    }
};
#endif