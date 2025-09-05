#ifndef TONEMAPPER_H
#define TONEMAPPER_H
#include "Vec3.hpp"
#include <algorithm>
#include <cmath>

enum class ToneMappingType {
    Reinhard,
    ACESFilm,
    Uncharted2
};

template<typename T = float>
class ToneMapper {
    ToneMappingType type;
    T L_mid;   // 中间灰亮度 cd/m²
    /*
    L_mid = 20 → 模拟昏暗室内
    L_mid = 200 → 模拟室外晴天
    L_mid = 2000 → 模拟雪地 / 太阳直射
    */
public:
    ToneMapper(ToneMappingType t = ToneMappingType::ACESFilm, T mid = 50) 
        : type(t), L_mid(mid) {}

    Vec3<T> map(const Vec3<T>& hdr) const {
        Vec3<T> x = hdr * (T(0.18) / L_mid); // 曝光调节

        switch(type) {
            case ToneMappingType::Reinhard:
                return reinhard(x);
            case ToneMappingType::ACESFilm:
                return acesFilm(x);
            case ToneMappingType::Uncharted2:
                return uncharted2(x);
        }
        return x; // 默认不变
    }

private:
    Vec3<T> reinhard(const Vec3<T>& x) const {
        return { x.x / (1 + x.x), x.y / (1 + x.y), x.z / (1 + x.z) };
    }

    Vec3<T> acesFilm(const Vec3<T>& x) const {
        auto f = [](T v) {
            T a = v*(v + 0.0245786f) - 0.000090537f;
            T b = v*(0.983729f*v + 0.4329510f) + 0.238081f;
            return std::clamp(a/b, T(0), T(1));
        };
        return { f(x.x), f(x.y), f(x.z) };
    }

    Vec3<T> uncharted2(const Vec3<T>& x) const {
        // Uncharted2 / John Hable
        auto tonemap = [](T v) {
            const T A=0.15f, B=0.50f, C=0.10f, D=0.20f, E=0.02f, F=0.30f;
            T num = ((v*(A*v + C*B) + D*E));
            T den = (v*(A*v + B) + D*F);
            return std::clamp(num/den - E/F, T(0), T(1));
        };
        return { tonemap(x.x), tonemap(x.y), tonemap(x.z) };
    }
};

#endif