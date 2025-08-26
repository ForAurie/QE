#ifndef VEC3_H
#define VEC3_H
#include <cmath>
#include <algorithm>
template<typename T = float>
struct Vec3 {
    T x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(T __x, T __y, T __z) : x(__x), y(__y), z(__z) {}
    Vec3(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}
    Vec3& operator=(const Vec3& v) {
        if (this != &v) x = v.x, y = v.y, z = v.z;
        return *this;
    }
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3& operator*=(const Vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    Vec3 operator/(const Vec3& v) const { return Vec3(x / v.x, y / v.y, z / v.z); }
    Vec3& operator/=(const Vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

    Vec3 operator*(T s) const { return Vec3(x * s, y * s, z * s); }
    Vec3& operator*=(T s) { x *= s; y *= s; z *= s; return *this; }
    Vec3 operator/(T s) const { return Vec3(x / s, y / s, z / s); }
    Vec3& operator/=(T s) { x /= s; y /= s; z /= s; return *this; }
    Vec3 operator+(T s) const { return Vec3(x + s, y + s, z + s); }
    Vec3& operator+=(T s) { x += s; y += s; z += s; return *this; }
    Vec3 operator-(T s) const { return Vec3(x - s, y - s, z - s); }
    Vec3& operator-=(T s) { x -= s; y -= s; z -= s; return *this; }
    Vec3& set(T __x, T __y, T __z) { x = __x; y = __y; z = __z; return *this; }
    constexpr inline Vec3 cross(const Vec3& v) const {
        return Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
    constexpr inline Vec3& crossSelf(const Vec3& v) {
        return *this = Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
    constexpr inline T dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    constexpr inline T length() const { return std::sqrt(x * x + y * y + z * z); }   
    // sqrt 的 constexpr 要求 C++ 版本 >= 20
    constexpr inline T lengthSquared() const { return x * x + y * y + z * z; } 
    const inline Vec3 normalized() const {
        const T len = std::sqrt(x * x + y * y + z * z);
        return len > 0 ? Vec3(x / len, y / len, z / len) : Vec3();
    }
    constexpr inline Vec3& normalizeSelf() {
        const T len = std::sqrt(x * x + y * y + z * z);
        if (len > 0) { x /= len; y /= len; z /= len; }
        else x = y = z = 0;
        return *this;
    }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    T max() const { return std::max({ x, y, z }); }
    T min() const { return std::min({ x, y, z }); }
};
template<typename T>
Vec3<T> operator*(T s, const Vec3<T>& v) { return Vec3<T>(v.x * s, v.y * s, v.z * s); }
template<typename T>
Vec3<T> operator+(T s, const Vec3<T>& v) { return Vec3<T>(v.x + s, v.y + s, v.z + s); }
template<typename T>
Vec3<T> operator-(T s, const Vec3<T>& v) { return Vec3<T>(s - v.x, s - v.y, s - v.z); }
#endif