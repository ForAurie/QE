#include <iostream>
#include <memory>
#include <climits>
#include <vector>
#include "QE.cpp"
#include "BMP.cpp"
#include "Camera.hpp"
#include <algorithm>
#include "ToneMapper.hpp"
#include <random>
using namespace std;
/*
DisplayHDR 400 ‌：要求全屏峰值亮度≥400 cd/m²，持续亮度≥320 cd/m²。
DisplayHDR 600 ‌：要求全屏峰值亮度≥600 cd/m²，持续亮度≥350 cd/m²，同时需支持99% BT.709色域 和≥90% DCI-P3色域 。
DisplayHDR 1000 ‌：峰值亮度≥1000 cd/m²，长续亮度≥600 cd/m²，角亮度≤0.05 cd/m²（需支持局部调光）。
实际应用场景
‌中国HDR认证 ‌：要求LCD显示器峰值亮度≥500 cd/m²，高端机型可达750 cd/m²以上。 ‌
HDR10 ‌：作为通用标准，通常要求峰值亮度≥400 cd/m²。 ‌
当前市场主流显示器多集中在400-750 cd/m²区间，高端游戏或专业设计显示器可能达到1000 cd/m²以上。 ‌

‌SDR APL 100%‌（全白画面）：典型值为300 cd/m²，最小值240 cd/m²。该亮度足够满足普通室内环境下的网页浏览、文档编辑等需求。 ‌
视频播放场景
‌SDR APL 10%‌（暗场中的中等高亮区域）：典型值未明确，但结合HDR参数推测，该场景下亮度可能达到500 cd/m²以上。 ‌
显示技术差异
不同厂商的SDR显示器亮度存在差异，例如华硕ROG XG27UQ绝影游戏显示器典型亮度为350 nit（约350 cd/m²），而苹果设备支持的SDR最大亮度为100 nit（约100 cd/m²）。 ‌
当前主流SDR显示器普遍采用300-400 cd/m²的亮度设计，既能保证日常使用足够亮，又不会因过高亮度造成视觉疲劳
*/
/*
👉 建议在渲染后做 色调映射 (tone mapping)，而不是简单线性裁剪。
例如用 Reinhard：

Vec3<float> mapped = colorOpt.value() / (colorOpt.value() + Vec3<float>(1.0f))
*/


ToneMapper<float> tmx(ToneMappingType::ACESFilm, 100);

Pixel toPixel(const std::optional<Vec3<float>>& colorOpt) {
    if (!colorOpt) return Pixel(0, 0, 0);
    // auto output = tmx.map(colorOpt.value());
    // return Pixel(std::min(255, (int)(output.x * 255)), std::min(255, (int)(output.y * 255)), std::min(255, (int)(output.z * 255)));
    const int mx = 150;
    return Pixel(std::min(255, (int)(colorOpt->x / mx * 255)),std::min(255, (int)(colorOpt->y / mx * 255)),std::min(255, (int)(colorOpt->z / mx * 255)));
}
/*
Vec3<T> albedo;      // 漫反射颜色
Vec3<T> F0;          // 基反射率 (non-metal: ~0.04, metal: 根据材质)
Vec3<T> IOR;         // 折射，进入材质内部的光比例
Vec3<T> refractiveIndex; // 折射率
T roughness;         // 粗糙度 [0,1]
T metalness;         // 金属度 [0,1]
T sigma;             // 材质浑浊度
* **Albedo（漫反射基色，非金属才有用）**
* **F0（基反射率，金属决定镜面颜色，非金属一般固定 \~0.04）**
* **Metalness（金属度，0=绝缘体，1=金属）**
* **Roughness（粗糙度，决定高光锐利度）**

---

## 📌 常见材质参数表

| 材质     | Albedo (RGB 0–1) | F0 (RGB)           | Metalness | Roughness (推荐范围) | 说明         |
| ------ | ---------------- | ------------------ | --------- | ---------------- | ---------- |
| **石头** | (0.3, 0.3, 0.3)  | (0.04, 0.04, 0.04) | 0.0       | 0.6–0.9          | 暗灰，漫反射主导   |
| **塑料** | (0.8, 0.1, 0.1)  | (0.04, 0.04, 0.04) | 0.0       | 0.3–0.7          | 漫反射主导，少量高光 |
| **木头** | (0.5, 0.35, 0.2) | (0.04, 0.04, 0.04) | 0.0       | 0.4–0.7          | 漫反射主导      |
| **陶瓷** | (0.9, 0.9, 0.9)  | (0.04, 0.04, 0.04) | 0.0       | 0.1–0.3          | 光滑亮白表面     |
| **铜**  | (0, 0, 0)        | (0.95, 0.64, 0.54) | 1.0       | 0.2–0.5          | 无漫反射，彩色反射  |
| **金**  | (0, 0, 0)        | (1.00, 0.77, 0.34) | 1.0       | 0.2–0.5          | 经典金属光泽     |
| **铝**  | (0, 0, 0)        | (0.91, 0.92, 0.92) | 1.0       | 0.05–0.3         | 银白亮反射      |
| **铁**  | (0, 0, 0)        | (0.56, 0.57, 0.58) | 1.0       | 0.3–0.7          | 偏灰金属，暗淡反射  |
| **银**  | (0, 0, 0)        | (0.95, 0.93, 0.88) | 1.0       | 0.05–0.3         | 明亮高反射      |

---

## 🌟 使用要点

* **非金属**：

  * `F0` 固定在 `(0.04,0.04,0.04)`（白色小反射）。
  * `albedo` 决定颜色。
* **金属**：

  * `albedo = (0,0,0)`（因为光都进入 specular）。
  * `F0` 决定反射颜色（查表）。
  * `metalness = 1.0`。
*/
int main() {
    Vec3<float> p0(0, 0, 0), p1(-1, 0, 0), p2(0, 0, -1), p3(-1, 0, -1), p4(-1, 1, 0), p5(-1, 1, -1), p6(0, 1, -1), p7(0, 1, 0);
    auto mat = make_unique<CookTorrancePBRMaterial<float>>(Vec3<float>(0.2, 0.3, 0.4), Vec3<float>(0.91, 0.92, 0.92), 0.35, 1, 0,
    nullptr, nullptr, nullptr, nullptr, nullptr
                                                           );
    // auto mat = make_unique<CookTorranceMaterial<float>>(Vec3<float>(0, 0, 0), Vec3<float>(1, 1, 1), Vec3<float>(0, 0, 0), Vec3<float>(1.5 , 1.5, 1.5), 0.1, 0, 0.1);
    auto matv = make_unique<MaterialSet<float>>(vector<pair<Material<float>*, float>>(
        {
            make_pair(mat.get(), 1)
        }
    ), false);
    auto mesh = make_unique<TriangleMesh<float>>(vector<Vec3<float>>({p0, p1, p2, p3, p4, p5, p6, p7}));
    
    mesh->insertTriangle(0, 1, 3, matv.get());
    mesh->insertTriangle(0, 3, 2, matv.get());

    mesh->insertTriangle(1, 4, 5, matv.get());
    mesh->insertTriangle(1, 5, 3, matv.get());

    mesh->insertTriangle(3, 5, 6, matv.get());
    mesh->insertTriangle(3, 6, 2, matv.get());

    mesh->insertTriangle(0, 4, 1, matv.get());
    mesh->insertTriangle(0, 7, 4, matv.get());

    mesh->insertTriangle(7, 5, 4, matv.get());
    mesh->insertTriangle(7, 6, 5, matv.get());

    mesh->insertTriangle(0, 2, 6, matv.get());
    mesh->insertTriangle(0, 6, 7, matv.get());

    auto light1 = make_unique<PointLight<float>>(Vec3<float>(2, 2, 2), Vec3<float>(5000, 5000, 5000));
    auto light2 = make_unique<PointLight<float>>(Vec3<float>(-3, 2, -3), Vec3<float>(5000, 5000, 5000));
    auto light3 = make_unique<TriangleLight<float>>(Vec3<float>(2, 0, 2), Vec3<float>(0, 2, 3), Vec3<float>(3, 2, 0), Vec3<float>(5000, 5000, 5000));
    Engine<float> engine;
    mesh->init();
    engine.insertInstance(Instance<float>(mesh.get(), Vec3<float>(0, 0, 0)));
    engine.insertInstance(Instance<float>(mesh.get(), Vec3<float>(-0.5, 0.1, 0.5)));
    engine.insertLight(light1.get());
    engine.insertLight(light2.get());
    engine.insertLight(light3.get());
    const size_t width = 1920, height = 1080;
    Camera<float> camera(Vec3<float>(2, 2, 2), Vec3<float>(-0.5, 0.5, -0.5), Vec3<float>(0, 1, 0), 90.0f * acos(-1) / 180.0f, width, height);
    engine.init();
    vector<vector<Pixel>> image(height, vector<Pixel>(width));
    uint64_t seed = 99832;
    mt19937 rng(static_cast<std::mt19937::result_type>(seed));
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            Ray<float> ray = camera.generateRay(i, j);
            auto colorOpt = engine.renderPixel(rng, ray, 0.05, 50);
            if (colorOpt)
                image[i][j] = toPixel(colorOpt);
        }
    }
    saveBMP("output.bmp", image);
    return 0;
}
/*
std::uint64_t seed = std::hash<float>{}(closestHit->position.x) ^ (std::hash<float>{}(closestHit->position.y) << 1);
std::mt19937 rng(static_cast<std::mt19937::result_type>(seed));
*/