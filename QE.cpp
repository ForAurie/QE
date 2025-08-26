#ifndef QE_CPP
#define QE_CPP
#include <vector>
#include <memory>
#include <optional>
#include <cmath>
#include <algorithm>
#include "Consts.hpp"
#include "Vec3.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Object.hpp"
#include "Ray.hpp"

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
template<typename T = float>
class Engine {
public:
    std::vector<std::shared_ptr<Object<T>>> objects; // 场景物体
    std::vector<std::shared_ptr<Light<T>>> lights;   // 场景光源
    void insertObject(const std::shared_ptr<Object<T>>& obj) { objects.push_back(obj); }
    void insertLight(const std::shared_ptr<Light<T>>& light) { lights.push_back(light); }
    std::optional<Vec3<T>> renderPixel(const Ray<T>& ray, const T sigma = 0.05f, const Vec3<T>& ambientLight = Vec3<T>(50, 50, 50), const size_t deep = 1) const {
        std::optional<HitInfo<T>> closestHit;
        T minT = std::numeric_limits<T>::infinity();
        for (const auto& obj : objects) {
            auto hit = obj->intersect(ray);
            if (hit && hit->t < minT) {
                minT = hit->t;
                closestHit = hit;
            }
        }
        if (!closestHit) return std::nullopt;
        const Material<T>* mat = closestHit->material;
        if (mat->type == MaterialType::selfIllumination) // 自发光材质
            return mat->reflectivity / (closestHit->t * closestHit->t); // 距离衰减
        Vec3<T> color(0, 0, 0);
        for (const auto& tmp : lights) {
            if (tmp->getType() != LightType::Point) continue; // 目前只支持点光源
            const PointLight<T>* light = dynamic_cast<const PointLight<T>*>(tmp.get());
            Vec3<T> toLight = light->position - closestHit->position;
            const T len2 = toLight.lengthSquared();
            const T len = std::sqrt(len2);
            toLight /= len;
            const Ray<T> shadowRay(closestHit->position + closestHit->normal * EPSILON, toLight); // 偏移以防自阴影
            if (std::any_of(objects.begin(), objects.end(), [&](const std::shared_ptr<Object<T>>& obj) {
                auto shadowHit = obj->intersect(shadowRay);
                return shadowHit && shadowHit->t < len;
            })) continue; // 阴影遮挡，跳过该光源
            const T diff = std::max(T(0), closestHit->normal.dot(toLight)); // 漫反射着色器（Diffuse Shader）
            const T ref = std::pow(std::max(T(0), (2 * closestHit->normal.dot(toLight) * closestHit->normal - toLight).dot(-ray.direction)), mat->shininess); // 镜面反射着色器（Specular Shader）
            const T attenuation = std::exp(-sigma * closestHit->t) / len2; // 空气吸收 & 平方比衰减
            color += (diff * mat->diffuseStrength + ref * (1.0f - mat->diffuseStrength)) * attenuation * light->color;
        }
        color += ambientLight; // 环境光
        color *= mat->reflectivity; // 反射率
        // 反射和折射暂不实现
        // if (tri->transparency.max() > EPSILON && deep > 0) {
        // }
        return color;
    }
};
#endif