#ifndef LIGJT_H
#define LIGHT_H
#include "Vec3.hpp"
enum LightType {
    Point,
    Directional,
    Spot
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
    Vec3<T> position, color;    // 光源颜色（强度）
    PointLight(const Vec3<T>& pos, const Vec3<T>& col) : position(pos), color(col) {}
    LightType getType() const override { return LightType::Point; }
};
#endif