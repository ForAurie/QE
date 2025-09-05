#ifndef RAY_H
#define RAY_H
#include "Vec3.hpp"
// 射线结构体
template<typename T = float>
struct Ray {
    Vec3<T> origin, direction;
    Ray(const Vec3<T>& o, const Vec3<T>& d) : origin(o), direction(d.normalized()) {}
};

#endif