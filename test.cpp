#include <iostream>
#include <memory>
#include <climits>
#include <vector>
#include "QE.cpp"
#include "BMP.cpp"

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
Pixel toPixel(const std::optional<Vec3<float>>& colorOpt) {
    if (!colorOpt) return Pixel(0, 0, 0);
    const int mx = 300; // SDR显示器典型亮度300 cd/m²
    return Pixel(std::min(int(colorOpt->x / mx * 255), 255), std::min(int(colorOpt->y / mx * 255), 255), std::min(int(colorOpt->z / mx * 255), 255));
}

int main() {
    Vec3<float> p0(0, 0, 0), p1(-1, 0, 0), p2(0, 0, -1), p3(-1, 0, -1), p4(-1, 1, 0), p5(-1, 1, -1), p6(0, 1, -1), p7(0, 1, 0);
    Triangle<float> tri1(p0, p1, p3, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);
    Triangle<float> tri2(p0, p3, p2, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);

    Triangle<float> tri3(p1, p4, p5, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);
    Triangle<float> tri4(p1, p5, p3, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);

    Triangle<float> tri5(p3, p5, p6, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);
    Triangle<float> tri6(p3, p6, p2, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);

    Triangle<float> tri7(p0, p4, p1, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);
    Triangle<float> tri8(p0, p7, p4, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);

    Triangle<float> tri9(p7, p5, p4, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);
    Triangle<float> tri10(p7, p6, p5, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);

    Triangle<float> tri11(p0, p2, p6, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);
    Triangle<float> tri12(p0, p6, p7, false, false, Vec3<float>(0.5, 0.4, 0.9), Vec3<float>(0, 0, 0), 0.65, 1.5, 64);

    TriangleMesh<float> mesh({tri1, tri2, tri3, tri4, tri5, tri6, tri7, tri8, tri9, tri10, tri11, tri12});
    PointLight<float> light(Vec3<float>(2, 2, 2), Vec3<float>(10000, 10000, 10000));

    Engine<float> engine;
    engine.insertObject(make_shared<TriangleMesh<float>>(mesh));
    engine.insertLight(make_shared<PointLight<float>>(light));
    const size_t width = 1920, height = 1080;
    Camera<float> camera(Vec3<float>(2, 2, 2), Vec3<float>(-0.5, 0.5, -0.5), Vec3<float>(0, 1, 0), 90.0f * acos(-1) / 180.0f, width, height);
    
    vector<vector<Pixel>> image(height, vector<Pixel>(width));
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            Ray<float> ray = camera.generateRay(i, j);
            auto colorOpt = engine.renderPixel(ray, 0.05, Vec3<float>(100, 100, 100), 1);
            if (colorOpt)
                image[i][j] = toPixel(colorOpt);
        }
    }
    saveBMP("output.bmp", image);
    return 0;
}
