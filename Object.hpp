#ifndef OBJECT_H
#define OBJECT_H
#include "Ray.hpp"
#include "Material.hpp"
#include "Vec3.hpp"
#include "Consts.hpp"

// enum ObjectType {
//     Triangle
//     IndexedTriangle,
//     TriangleMesh
// };

// 物体基类
template<typename T = float>
class Object {
public:
    const Material<T>* material;
    Object(const Material<T>* mat = nullptr) : material(mat) {}
    virtual ~Object() = default;
    virtual std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const = 0; // 判断射线是否与物体相交，返回交点信息
};

template<typename T = float>
class Triangle : public Object<T> {
public:
    Vec3<T> v0, v1, v2; // 三个顶点
    Vec3<T> edge1, edge2, normal; // 预处理边和法线
    Triangle(
        const Vec3<T>& a = Vec3<T>(),
        const Vec3<T>& b = Vec3<T>(),
        const Vec3<T>& c = Vec3<T>(),
        const Material<T>* mat = nullptr
    ) :
        Object<T>(mat),
        v0(a), v1(b), v2(c),
        edge1(b - a),
        edge2(c - a)
    {
        normal = edge1.cross(edge2).normalized();
    }
    // Möller–Trumbore 算法实现射线与三角形求交
    std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const override {
        Vec3<T> h = ray.direction.cross(edge2);
        T a = edge1.dot(h);
        if (std::abs(a) < EPSILON) return std::nullopt; // 平行或退化
        if (!(this->material->doubleSided) && a < 0) return std::nullopt; // 单面剔除
        T f = 1 / a;
        Vec3<T> s = ray.origin - v0;
        T u = f * s.dot(h);
        if (u < 0 || u > 1) return std::nullopt;
        Vec3<T> q = s.cross(edge1);
        T v = f * ray.direction.dot(q);
        if (v < 0 || u + v > 1) return std::nullopt;
        T t = f * edge2.dot(q);
        if (t < EPSILON) return std::nullopt; // 交点在射线起点之后
        // 如果是背面命中且允许双面渲染，法线取反
        return HitInfo<T>{ t, ray.origin + ray.direction * t, a < 0 ? -normal : normal , this->material };
    }
};

template<typename T>
class TriangleMesh;

// 三角形类，存索引而不是顶点
template<typename T = float>
class IndexedTriangle : public Object<T> {
public:
    size_t v0, v1, v2;          // 顶点索引
    Vec3<T> edge1, edge2, normal;
    const TriangleMesh<T>* mesh; // 指向所属 Mesh，访问顶点
    IndexedTriangle(size_t a, size_t b, size_t c,
                    const TriangleMesh<T>* m,
                    const Material<T>* mat = nullptr)
        : Object<T>(mat), v0(a), v1(b), v2(c), mesh(m)
    {
        computeEdges();
    }
    void computeEdges() {
        edge1 = mesh->points[v1] - mesh->points[v0];
        edge2 = mesh->points[v2] - mesh->points[v0];
        normal = edge1.cross(edge2).normalized();
    }
    std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const override {
        Vec3<T> h = ray.direction.cross(edge2);
        T a = edge1.dot(h);
        if (std::abs(a) < EPSILON) return std::nullopt; // 平行或退化
        if (!(this->material->doubleSided) && a < 0) return std::nullopt; // 单面剔除
        T f = 1 / a;
        Vec3<T> s = ray.origin - mesh->points[v0];
        T u = f * s.dot(h);
        if (u < 0 || u > 1) return std::nullopt;
        Vec3<T> q = s.cross(edge1);
        T v = f * ray.direction.dot(q);
        if (v < 0 || u + v > 1) return std::nullopt;
        T t = f * edge2.dot(q);
        if (t < EPSILON) return std::nullopt; // 交点在射线起点之后
        // 背面命中取反法线
        return HitInfo<T>{ t, ray.origin + ray.direction * t, a < 0 ? -normal : normal, this->material };
    }
};
// 基于三角形网格的Object类（可用于加载复杂模型）
template<typename T = float>
class TriangleMesh : public Object<T> {
public:
    std::vector<Vec3<T>> points;
    std::vector<IndexedTriangle<T>> triangles;
    std::vector<std::vector<size_t>> mp;
    TriangleMesh(const std::vector<Vec3<T>>& points): points(points), mp(points.size()) {};
    void insertPoint(const Vec3<T>& p) { points.push_back(p); mp.push_back(std::vector<size_t>()); }
    void insertTriangle(size_t a, size_t b, size_t c, const Material<T>* mat) { triangles.emplace_back(a, b, c, this, mat); mp[a].push_back(triangles.size() - 1); mp[b].push_back(triangles.size() - 1); mp[c].push_back(triangles.size() - 1); }
    void update(size_t idx, const Vec3<T>& p) {
        points[idx] = p;
        for (const auto i : mp[idx]) {
            triangles[i].computeEdges();
        }
    }
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
#endif