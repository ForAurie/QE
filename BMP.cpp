#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
namespace BMP {
#pragma pack(push, 1)  // 确保按字节对齐

// 定义 BMP 文件头结构
struct BMPFileHeader {
    uint16_t bfType;        // 文件类型，通常为 'BM'
    uint32_t bfSize;        // 文件大小
    uint16_t bfReserved1;   // 保留字段
    uint16_t bfReserved2;   // 保留字段
    uint32_t bfOffBits;     // 从文件头到像素数据的偏移量
};

// 定义 BMP 信息头结构
struct BMPInfoHeader {
    uint32_t biSize;        // 信息头大小
    int32_t biWidth;        // 图像宽度
    int32_t biHeight;       // 图像高度
    uint16_t biPlanes;      // 颜色平面数
    uint16_t biBitCount;    // 每个像素的位数
    uint32_t biCompression; // 压缩类型（0 表示无压缩）
    uint32_t biSizeImage;   // 图像大小
    int32_t biXPelsPerMeter; // 水平分辨率
    int32_t biYPelsPerMeter; // 垂直分辨率
    uint32_t biClrUsed;     // 使用的颜色数
    uint32_t biClrImportant; // 重要颜色数
};

// 定义像素结构
struct Pixel {
    uint8_t blue;   // 蓝色分量
    uint8_t green;  // 绿色分量
    uint8_t red;    // 红色分量

    // 构造函数，初始化颜色值
    Pixel(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : blue(b), green(g), red(r) {}
};

#pragma pack(pop) // 恢复对齐方式

// 保存 BMP 文件
void saveBMP(const std::string& filename, const std::vector<std::vector<Pixel>>& image) {
    int width = image[0].size();   // 假设每行宽度相同
    int height = image.size();

    // 计算文件大小
    int rowSize = (width * 3 + 3) / 4 * 4;  // 每行的字节数（按 4 字节对齐）
    int imageSize = rowSize * height;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fileHeader.bfType = 0x4D42;  // 'BM'
    fileHeader.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24; // 每个像素 24 位（RGB）
    infoHeader.biCompression = 0; // 无压缩
    infoHeader.biSizeImage = imageSize;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    std::ofstream file(filename, std::ios::binary);

    // 写文件头
    file.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    // 写信息头
    file.write(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    // 写像素数据
    for (int i = height - 1; i >= 0; --i) {
        for (int j = 0; j < width; ++j) {
            // 获取像素的 RGB 数据
            const Pixel& p = image[i][j];
            file.put(p.blue).put(p.green).put(p.red);
        }

        // 填充每行的对齐字节
        for (int j = 0; j < (rowSize - width * 3); ++j) {
            file.put(0);
        }
    }

    file.close();
}
}
using BMP::Pixel;
using BMP::saveBMP;
