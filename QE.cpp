#ifndef QE_H
#define QE_H
#include <vector>
#include <memory>
#include <optional>
#include <cmath>
#include <algorithm>
// #include "Consts.hpp"
#include "Vec3.hpp"
#include "Ray.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "BVH.hpp"
#include "Object.hpp"
#include <random>
/*
漫反射着色器（Diffuse Shader）
表现物体表面对光线的均匀反射（如粉笔、墙壁等无光泽表面）。
最常用的是 Lambert 漫反射模型。

镜面反射着色器（Specular Shader）
表现物体表面的高光反射（如金属、塑料等有光泽表面）。
常用模型有 Phong、Blinn-Phong、Cook-Torrance 等。

环境光着色器（Ambient Shader）
表现环境中无方向的基础光照，防止阴影处完全黑色。
通常为一个常量或全局光照项。

反射/折射着色器（Reflection/Refraction Shader）
用于表现镜面反射、透明折射等效果。
需要递归追踪反射/折射光线。

自发光着色器（Emission Shader）
物体本身发光，不受外部光源影响。
*/
#include <cassert>
template<typename T = float>
class Engine {
public:
    std::vector<Instance<T>> instances; // 场景物体
    std::vector<Light<T>*> lights;   // 场景光源
    TLAS<T> tlas;
    Engine(
        const std::vector<Instance<T>>& __instances = std::vector<Instance<T>>(),
        const std::vector<Light<T>*>& __lights = std::vector<Light<T>*>()
    ) : instances(__instances), lights(__lights), tlas() {}
    void insertInstance(const Instance<T>& ins) { instances.push_back(ins); }
    void insertLight(Light<T>* light) { lights.push_back(light); }
    void init() { tlas.build(instances); }
    template<typename URNG>
    std::optional<Vec3<T>> renderPixel(URNG& rng, const Ray<T>& ray, const T sigma = 0.05f, const int TRI_LIGHT_SPP = 5, const size_t deep = 2) const {
        std::optional<HitInfo<T>> closestHit = tlas.intersect(ray);
        if (!closestHit) return std::nullopt;
        Vec3<T> color(0, 0, 0);
        for (const auto& tmp : lights) {
            switch(tmp->getType()) {
            case LightType::Point:{
                const PointLight<T>* light = dynamic_cast<const PointLight<T>*>(tmp);
                Vec3<T> toLight = light->position - closestHit->position;
                const T len2 = toLight.lengthSquared();
                if (len2 < T(0)) continue;
                const T len = std::sqrt(len2);
                toLight /= len;

                const Ray<T> shadowRay(closestHit->position + closestHit->normal * EPSILON, toLight); // 偏移以防自阴影
                auto shadownHit = tlas.intersect(shadowRay);
                if (shadownHit && shadownHit->t < len) continue; // 阴影遮挡，跳过该光源
                const Vec3<T> input = light->color / len2;
                for (const auto& material : *closestHit->materialSet)
                    color += material.second * material.first->getColor(input, -ray.direction, toLight, closestHit->normal, 0, 0);
                break;
            }
            case LightType::Triangle:{
                if (TRI_LIGHT_SPP <= 0) continue;
                const TriangleLight<T>* light = dynamic_cast<const TriangleLight<T>*>(tmp);
                Vec3<T> sum(0,0,0);
                if (light->area <= T(0)) continue;
                for (int i = 0; i < TRI_LIGHT_SPP; ++i) {
                    // 1) 采样光源面一点
                    auto position = light->samplePoint(rng);
                    // 2) 方向/距离
                    Vec3<T> toLight = position - closestHit->position;
                    const T len2 = toLight.lengthSquared();
                    if (len2 <= T(0)) continue;
                    const T len  = std::sqrt(len2);
                    toLight /= len;
                    const T cosL = light->normal.dot(-toLight);
                    if (cosL <= T(0)) continue;
                    // 3) 可见性：阴影测试（距离裁剪）
                    const Ray<T> shadowRay(closestHit->position + closestHit->normal * EPSILON, toLight);
                    auto shadownHit = tlas.intersect(shadowRay);
                    if (shadownHit && shadownHit->t < len) continue; // 阴影遮挡，跳过该光源
                    // 4) NEE 权重：Li * (cosL) / (dist^2 * pdfA)
                    // 其中 Li = light->emission（radiance，常量）
                    // getColor 内部会再乘一次 NdotL（接收端），等效得到 f * Li * NdotL * cosL / (dist^2 * pdfA)
                    const Vec3<T> input = light->color * light->area * cosL / len2;
                    for (const auto& material : *closestHit->materialSet)
                        sum += material.second * material.first->getColor(input, -ray.direction, toLight, closestHit->normal, 0, 0);
                }
                // 多重采样均值
                color += sum / T(TRI_LIGHT_SPP);
                break;
            }
            default:
            break;
            }
        }
        color *= std::exp(-sigma * closestHit->t);
        // 反射和折射暂不实现
        // if (tri->transparency.max() > EPSILON && deep > 0) {

        // }
        return color;
    }
};
#endif