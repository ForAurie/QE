#ifndef MATERIAL_H
#define MATERIAL_H
#include "Vec3.hpp"
enum class MaterialType {
    common,
    selfIllumination,
    Texture
};
template<typename T>
class Material {
public:
    MaterialType type;
    bool doubleSided;    // 是否渲染背面，暂不支持双面渲染
    Vec3<T> reflectivity; // 反射率
    Vec3<T> transparency; // 透明度
    T refractiveIndex; // 折射率，如果分三种颜色的折射率了话计算量太大，1=空气, 1.33=水, 1.5=玻璃, 2.42=钻石
    T diffuseStrength;       // 漫反射强度 [0,1], 0=全镜面反射, 1=全漫反射
    T shininess;      // 高光指数，越大高光越集中
    Material(
        MaterialType __type = MaterialType::common,
        bool __doubleSided = false,
        const Vec3<T>& __reflectivity = Vec3<T>(0.8, 0.8, 0.8),
        const Vec3<T>& __transparency = Vec3<T>(0, 0, 0),
        T __refractiveIndex = 1.5,
        T __diffuseStrength = 0.65,
        T __shininess = 64
    ) : type(__type), doubleSided(__doubleSided), reflectivity(__reflectivity), transparency(__transparency),
        refractiveIndex(__refractiveIndex), diffuseStrength(__diffuseStrength), shininess(__shininess) {}
};
// 光线与物体的交点信息
template<typename T = float>
struct HitInfo {
    T t = 0;          // 交点距离
    Vec3<T> position; // 交点坐标
    Vec3<T> normal;   // 法线
    const Material<T>* material = nullptr; // 材质信息
};
#endif