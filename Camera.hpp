#ifndef CAMERA_H
#define CAMERA_H

#include "Vec3.hpp"
#include "Consts.hpp"
#include <cmath>
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
           T __fovh = 90.0f * PI / 180.0f, size_t __width = 1920, size_t __height = 1080)
        : position(__position), lookAt(__lookAt), up(__up), width(__width), height(__height) {
            __fovh /= 2;
            tfovh = tan(__fovh);
            tfovw = tfovh / height * width;
            forward = (lookAt - position).normalized();
            right = forward.cross(up).normalized();
            down = -right.cross(forward).normalized();
        }
    Ray<T> generateRay(size_t y, size_t x) const {
        const T ndcX = (x + 0.5f) / width - 0.5, ndcY = (y + 0.5f) / height - 0.5;
        return Ray<T>(position, (forward + right * ndcX * tfovw + down * ndcY * tfovh).normalized());
    }
};
#endif