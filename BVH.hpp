#ifndef BVH_H
#define BVH_H
#include <vector>
#include <algorithm>
#include <limits>
#include <memory>
#include "Vec3.hpp"
#include "Ray.hpp"

// AABB 包围盒
template<typename T = float>
struct AABB {
    Vec3<T> min, max;
    AABB() : min(std::numeric_limits<T>::infinity()), max(-std::numeric_limits<T>::infinity()) {}
    AABB(const Vec3<T>& __min, const Vec3<T>& __max) : min(__min), max(__max) {}
    inline void expandMax(const Vec3<T>& p) {
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        max.z = std::max(max.z, p.z);
    }
    inline void expandMin(const Vec3<T>& p) {
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        min.z = std::min(min.z, p.z);
    }
    inline void expand(const Vec3<T>& p) {
        expandMax(p);
        expandMin(p);
    }
    inline void expand(const AABB& box) {
        expandMin(box.min);
        expandMax(box.max);
    }
    inline bool intersect(const Ray<T>& ray, T tMin = 0, T tMax = std::numeric_limits<T>::infinity()) const {
        for (int i = 0; i < 3; ++i) {
            T invD = T(1) / ray.direction[i];
            T t0 = (min[i] - ray.origin[i]) * invD;
            T t1 = (max[i] - ray.origin[i]) * invD;
            if (invD < 0) std::swap(t0, t1);
            tMin = std::max(t0, tMin);
            tMax = std::min(t1, tMax);
            if (tMax <= tMin) return false;
        }
        return true;
    }
};
template<typename T>
class Object;
template<typename T>
struct IndexedTriangle;

template<typename T = float>
class Instance {
public:
    Object<T>* object;
    Vec3<T> translation;
    Instance(Object<T>* __object, const Vec3<T>& __translation) : object(__object), translation(__translation){}
};
// ================================== BLAS ==================================
template<typename T = float>
struct BLASNode {
    AABB<T> box;
    std::unique_ptr<BLASNode> left;
    std::unique_ptr<BLASNode> right;
    std::vector<const Object<T>*> objects; // 叶子节点存三角形索引
    inline bool isLeaf() const { return left == nullptr && right == nullptr; }
};

template<typename T>
class BLAS {
private:
    // ================= BLAS 构建 =================
    std::unique_ptr<BLASNode<T>> __build(const std::vector<Vec3<T>>& points, std::vector<IndexedTriangle<T>>& triangles, std::vector<size_t>& indices, int depth = 0) {
        auto node = std::make_unique<BLASNode<T>>();
        // 1. 计算包围盒
        for (auto i : indices)
            node->box.expand(triangles[i].getAABB());
        // 2. 终止条件
        if (indices.size() <= 4 || depth > 20) {
            for (auto i : indices)
                node->objects.push_back(&triangles[i]);
            return node;
        }
        // 3. 选择最长轴排序
        const Vec3<T> extents = node->box.max - node->box.min;
        int axis = 0;
        if (extents.y > extents.x) axis = 1;
        if (extents.z > extents[axis]) axis = 2;
        auto mid = indices.size() / 2;
        std::nth_element(indices.begin(), indices.begin() + mid, indices.end(), [&](size_t a, size_t b) {
            return (points[triangles[a].v0][axis] + points[triangles[a].v1][axis] + points[triangles[a].v2][axis]) < (points[triangles[b].v0][axis] + points[triangles[b].v1][axis] + points[triangles[b].v2][axis]);
        });
        // 4. 分成两半
        std::vector<size_t> leftIndices(indices.begin(), indices.begin() + mid);
        std::vector<size_t> rightIndices(indices.begin() + mid, indices.end());
        node->left = __build(points, triangles, leftIndices, depth + 1);
        node->right = __build(points, triangles, rightIndices, depth + 1);
        return node;
    }
    // ================= BLAS 遍历 =================
    std::optional<HitInfo<T>> __intersect(const Ray<T>& ray, const BLASNode<T>* node) const {
        if (!node || !node->box.intersect(ray)) return std::nullopt;
        if (node->isLeaf()) {
            std::optional<HitInfo<T>> closestHit;
            T minT = std::numeric_limits<T>::infinity();
            for (auto obj : node->objects) {
                auto hit = obj->intersect(ray);
                if (hit && hit->t < minT) {
                    minT = hit->t;
                    closestHit = hit;
                }
            }
            return closestHit;
        }
        auto leftHit = __intersect(ray, node->left.get());
        auto rightHit = __intersect(ray, node->right.get());
        if (leftHit && rightHit)
            return leftHit->t < rightHit->t ? leftHit : rightHit;
        else if (leftHit) return leftHit;
        else return rightHit;
    }
public:
    std::unique_ptr<BLASNode<T>> root;
    BLAS() : root(nullptr) {}
    // ================= BLAS 构建 =================
    void build(const std::vector<Vec3<T>>& points, std::vector<IndexedTriangle<T>>& triangles) {
        std::vector<size_t> indices(triangles.size());
        for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;
        root = __build(points, triangles, indices);
    }
    inline std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const {
        return __intersect(ray, root.get());
    }
};


// ================================== TLAS ==================================
template<typename T = float>
struct TLASNode {
    AABB<T> box;
    std::unique_ptr<TLASNode> left;
    std::unique_ptr<TLASNode> right;
    Object<T>* object = nullptr;            // 叶子节点指向对象
    Vec3<T> translation = Vec3<T>(0, 0, 0); // 可扩展支持旋转/缩放
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};
template<typename T>
class TLAS {
public:
    std::unique_ptr<TLASNode<T>> root;
    // 每个 instance: object 指针 + 变换矩阵（这里先简单只做平移）
    TLAS() : root(nullptr) {}
    // ================= TLAS 构建 =================
    void build(const std::vector<Instance<T>>& instances) {
        std::vector<size_t> indices(instances.size());
        for (size_t i = 0; i < indices.size(); i++) indices[i] = i;
        root = __build(instances, indices);
    }
    inline std::optional<HitInfo<T>> intersect(const Ray<T>& ray) const {
        return __intersect(ray, root.get());
    }
private:
    // ================= TLAS 构建 =================
    std::unique_ptr<TLASNode<T>> __build(const std::vector<Instance<T>>& instances, std::vector<size_t>& indices) {
        auto node = std::make_unique<TLASNode<T>>();
        // 1. 计算 AABB
        for (auto idx : indices) {
            AABB<T> box = instances[idx].object->getAABB(); // 每个 Object 需提供 getAABB()
            // 平移
            box.min += instances[idx].translation;
            box.max += instances[idx].translation;
            node->box.expand(box);
        }
        // 2. 终止条件
        if (indices.size() == 1) {
            node->object = instances[indices.front()].object;
            node->translation = instances[indices.front()].translation;
            return node;
        }

        // 3. 找最长轴排序
        const Vec3<T> extents = node->box.max - node->box.min;
        int axis = 0;
        if (extents.y > extents.x) axis = 1;
        if (extents.z > extents[axis]) axis = 2;

        auto mid = indices.size() / 2;
        std::nth_element(indices.begin(), indices.begin() + mid, indices.end(), [&](size_t a, size_t b) {
            return  (instances[a].object->getAABB().min[axis] + instances[a].object->getAABB().max[axis]) * T(0.5) + instances[a].translation[axis]
                    <
                    (instances[b].object->getAABB().min[axis] + instances[b].object->getAABB().max[axis]) * T(0.5) + instances[b].translation[axis];
        });

        // 4. 分割
        std::vector<size_t> leftIndices(indices.begin(), indices.begin() + mid);
        std::vector<size_t> rightIndices(indices.begin() + mid, indices.end());
        node->left = __build(instances, leftIndices);
        node->right = __build(instances, rightIndices);
        return node;
    }
    // ================= TLAS 遍历 =================
    std::optional<HitInfo<T>> __intersect(const Ray<T>& ray, const TLASNode<T>* node) const {
        if (!node || !node->box.intersect(ray)) return std::nullopt;
        if (node->isLeaf()) {
            // 将光线变换到对象局部空间
            Ray<T> localRay = ray;
            localRay.origin -= node->translation;
            auto hit = node->object->intersect(localRay);
            if (hit) hit->position += node->translation; // 转回世界空间
            return hit;
        }

        auto leftHit = __intersect(ray, node->left.get());
        auto rightHit = __intersect(ray, node->right.get());

        if (leftHit && rightHit)
            return leftHit->t < rightHit->t ? leftHit : rightHit;
        else if (leftHit) return leftHit;
        else return rightHit;
    }
};
#endif
