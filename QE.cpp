#ifndef QE_CPP
#define QE_CPP
#include <vector>
#include <memory>
#include <optional>
#include <cmath>
#include <algorithm>
#include "Vec3"
constexpr float EPSILON = 1e-6;

// 射线结构体
template<typename T = float>
struct Ray {
    Vec3<T> origin, direction;
    Ray(const Vec3<T>& o, const Vec3<T>& d) : origin(o), direction(d.normalized()) {}
};
template<typename T>
struct HitInfo;
// 物体基类
template<typename T = float>
class Object {
public:
    virtual ~Object() = default;
    // 判断射线是否与物体相交，返回交点信息
    virtual std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const = 0;
};
// 光线与物体的交点信息
template<typename T = float>
struct HitInfo {
    T t = 0;          // 交点距离
    Vec3<T> position; // 交点坐标
    Vec3<T> normal;   // 法线
    const Object<T>* object = nullptr; // 交点所属物体
    // 可扩展材质等信息
};
template<typename T = float>
class Triangle : public Object<T> {
public:
    Vec3<T> v0, v1, v2; // 三个顶点
    bool doubleSided;    // 是否渲染背面，暂不支持双面渲染
    bool selfIllumination; // 是否自发光, 若为 true, 忽略光照计算，reflectivity 代表颜色
    Vec3<T> reflectivity; // 反射率
    Vec3<T> transparency; // 透明度
    T diffuseStrength;       // 漫反射强度 [0,1], 0=全镜面反射, 1=全漫反射
    T refractiveIndex; // 折射率，如果分三种颜色的折射率了话计算量太大，1=空气, 1.33=水, 1.5=玻璃, 2.42=钻石
    T shininess;      // 高光指数，越大高光越集中
    // bool useTexture;     // 是否使用贴图
    Triangle(
        const Vec3<T>& a, const Vec3<T>& b, const Vec3<T>& c,
        bool doubleSided = false,
        bool selfIllumination = false,
        const Vec3<T>& reflectivity = Vec3<T>(0.8, 0.8, 0.8),
        const Vec3<T>& transparency = Vec3<T>(0, 0, 0),
        T diffuseStrength = 0.65,
        T refractiveIndex = 1.5,
        T shininess = 32
    ) :
        v0(a), v1(b), v2(c),
        doubleSided(doubleSided),
        selfIllumination(selfIllumination),
        reflectivity(reflectivity),
        transparency(transparency),
        diffuseStrength(diffuseStrength),
        refractiveIndex(refractiveIndex),
        shininess(shininess)
    {}
    // Möller–Trumbore 算法实现射线与三角形求交
    std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const override {
        Vec3<T> edge1 = v1 - v0, edge2 = v2 - v0;
        Vec3<T> h = ray.direction.cross(edge2);
        T a = edge1.dot(h);
        if (std::abs(a) < EPSILON) return std::nullopt; // 平行或退化
        if (!doubleSided && a < 0) return std::nullopt; // 单面剔除
        T f = 1.0f / a;
        Vec3<T> s = ray.origin - v0;
        T u = f * s.dot(h);
        if (u < 0.0f || u > 1.0f) return std::nullopt;
        Vec3<T> q = s.cross(edge1);
        T v = f * ray.direction.dot(q);
        if (v < 0.0f || u + v > 1.0f) return std::nullopt;
        T t = f * edge2.dot(q);
        if (t < EPSILON) return std::nullopt; // 交点在射线起点之后
        // 如果是背面命中且允许双面渲染，法线取反
        return HitInfo<T>{ t, ray.origin + ray.direction * t, a < 0 ? -edge1.cross(edge2).normalized() : edge1.cross(edge2).normalized(), this };
    }
};
// 基于三角形网格的Object类（可用于加载复杂模型）
template<typename T = float>
class TriangleMesh : public Object<T> {
public:
    std::vector<Triangle<T>> triangles;
    TriangleMesh(const std::vector<Triangle<T>>& tris) : triangles(tris) {}
    std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const override {
        std::optional<HitInfo<T>> closestHit;
        T minT = std::numeric_limits<T>::infinity();
        for (const auto& tri : triangles) {
            auto hit = tri.intersect(ray);
            if (hit && hit->t < minT) {
                minT = hit->t;
                closestHit = hit;
            }
        }
        return closestHit;
    }
};
// 点光源
template<typename T = float>
class Light {
    public:
    virtual ~Light() = default;
};
template<typename T = float>
class PointLight : public Light<T> {
public:
    Vec3<T> position, color;    // 光源颜色（强度）
    PointLight(const Vec3<T>& pos, const Vec3<T>& col) : position(pos), color(col) {}
};
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
        if (closestHit) {
            const Triangle<T>* tri = dynamic_cast<const Triangle<T>*>(closestHit->object);
            if (!tri) return std::nullopt;
            if (tri->selfIllumination) // 自发光材质
                return tri->reflectivity / (closestHit->t * closestHit->t); // 距离衰减
            Vec3<T> color(0, 0, 0);
            for (const auto& tmp : lights) {
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
                const T ref = std::pow(std::max(T(0), (2 * closestHit->normal.dot(toLight) * closestHit->normal - toLight).dot(-ray.direction)), tri->shininess); // 镜面反射着色器（Specular Shader）
                const T attenuation = std::exp(-sigma * closestHit->t) / len2; // 空气吸收 & 平方比衰减
                color += (diff * tri->diffuseStrength + ref * (1.0f - tri->diffuseStrength)) * attenuation * light->color;
            }
            color += ambientLight; // 环境光
            color *= tri->reflectivity; // 反射率
            // 反射和折射暂不实现
            // if (tri->transparency.max() > EPSILON && deep > 0) {
            // }
            return color;
        }
        return std::nullopt;
    }
};
// #include <iostream>
// Camera类
template<typename T = float>
class Camera {
public:
    Vec3<T> position;            // 相机位置
    Vec3<T> lookAt;              // 相机朝向的目标点
    Vec3<T> up;                  // 相机的“上”方向（通常是Y轴）
    T tfovh, tfovw, width, height; // 视野角度（以弧度为单位）
    // T nearPlane, farPlane; // 最近和最远的裁剪平面

    Vec3<T> forward, right, down; // 相机坐标系基向量
    Camera(const Vec3<T>& __position, const Vec3<T>& __lookAt, const Vec3<T>& __up = Vec3<T>(0, 1, 0),
           T __fovh = 90.0f * acos(-1) / 180.0f, size_t __width = 1920, size_t __height = 1080)
        : position(__position), lookAt(__lookAt), up(__up), width(__width), height(__height) {
            __fovh /= 2;
            tfovh = tan(__fovh);
            tfovw = tfovh / height * width;
            forward = (lookAt - position).normalized();
            right = forward.cross(up).normalized();
            down = -right.cross(forward).normalized();
        }
    // Ray<T> generateRay(T x, T y) const {
    //     x += 0.5;
    //     y += 0.5;
    //     return Ray<T>(position, (forward + right * tan(2 * fovw * y / width - fovw) + down * tan(2 * fovh * x / height - fovh)).normalized());
    // }
    Ray<T> generateRay(size_t y, size_t x) const {
        const T ndcX = (x + 0.5f) / width - 0.5, ndcY = (y + 0.5f) / height - 0.5;
        return Ray<T>(position, (forward + right * ndcX * tfovw + down * ndcY * tfovh).normalized());
    }
};
#endif