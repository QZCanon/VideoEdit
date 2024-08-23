#include "utils.h"
#include <iostream>
#include "Logger/logger.h"

/**
 *
 * DJI action4，ffmpeg解码后y分量和uv分量的字节数相同，即2个y分量共享一组uv分量，采样比例为422
 *
 * p010le
 * yuv分量存储数据类型：uint8_t
 *  y分量：
 *      一个y分量占2字节
 *      字节序：低位在前，高为在后
 *      例：{11, 01, 22, 02 ...}
 *          =》 第一个y分量：0x0111
 *              第二个y分量：0x0222
 *
 *      u分量和v分量都占两个字节
 *      字节序：u分量低字节  u分量高字节  v分量低字节  v分量高字节
 *      例：{45, 03, 56, 04, 67, 05, 78, 06  ...}
 *          =》 第一个u分量：0x0345  第一个v分量：0x0456
 *              第二个u分量：0x0567  第二个v分量：0x0678
 *
 * 转换为yuv422p
 *      一个y分量占一个字节
 *      转换：yuv422p_y = p010le_y >> 2;
 *         即：uint8_t yuv422p_y = (p010le_data[0]) >> 2 | (p010le_data[1] << 6)
 *                                  (删[1:0]，空[7:6])        (高2位数据移植[7:6])
 *      uv分量转换同理
 *
 *
 */

// 假设图像的宽度和高度
#define WIDTH  1920
#define HEIGHT 1080

// 计算 YUV 数据在数组中的大小
#define Y_SIZE (WIDTH * HEIGHT)         // 每个 Y 值为 1 字节（8 位）
#define UV_SIZE ((WIDTH / 2) * (HEIGHT / 2) * 2) // UV 数据为 2 字节/分量

void convertP010LEToRGBA(uint8_t* yData, uint8_t* uvData, uint8_t* rgbaData,
                         int width, int height)
{
    int uvWidth = width / 4;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int yIndex = y * width * 2 + x * 2; // 2 bytes per Y value
            int uvIndex = (y * uvWidth + (x / 2)) * 4; // 2 bytes per U and V

            uint16_t yValue = (yData[yIndex] | (yData[yIndex + 1] << 8)) >> 6; // 10 bits Y
            uint16_t uValue = (uvData[uvIndex] | (uvData[uvIndex + 1] << 8)) >> 6; // 10 bits U
            uint16_t vValue = (uvData[uvIndex + 2] | (uvData[uvIndex + 3] << 8)) >> 6; // 10 bits V

            // YUV 转 RGB
            int r = (int)(yValue + 1.402 * (vValue - 512));
            int g = (int)(yValue - 0.344136 * (uValue - 512) - 0.714136 * (vValue - 512));
            int b = (int)(yValue + 1.772 * (uValue - 512));

            // Clip RGB 值到 [0, 255] 范围
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);

            // 存储 RGBA 数据
            int rgbaIndex = (y * width + x) * 4;
            rgbaData[rgbaIndex]     = (uint8_t)r;   // R
            rgbaData[rgbaIndex + 1] = (uint8_t)g;   // G
            rgbaData[rgbaIndex + 2] = (uint8_t)b;   // B
            rgbaData[rgbaIndex + 3] = 255;           // A (完全不透明)
        }
    }
}

// 将 P010LE 的 10 位 YUV 数据转换为 8 位 YUV 数据
void convertP010LETo8bit(uint8_t* p010leY,  const size_t ySize,
                         uint8_t* p010leUV, const size_t uvSize,
                         uint8_t* yData, uint8_t* uData, uint8_t* vData) {

    // std::cout << "ySize: " << ySize << ", uvSize: " << uvSize << std::endl;
    assert(ySize == uvSize);

    /*
    [Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7, Y8, Y9, Y10, Y11, Y12, Y13, Y14, Y15]
    [U0, U1, U2, U3, U4, U5, U6, U7]
    [V0, V1, V2, V3, V4, V5, V6, V7]
    */

    // 处理 Y 分量
    for (size_t i = 0; i < ySize; i += 2) {
        uint16_t nv12_y = ((uint16_t)(p010leY[i]) >> 2) |
                           ((uint16_t)(p010leY[i + 1]) << 6); // 提取 10 位 Y 分量
        yData[i/2] = (uint8_t)(nv12_y);
    }

    // // 处理uv分量
    // for (size_t i = 0, idx_ = 0; i < uvSize; i += 4, idx_++)
    // {
    //     uint16_t uVal = ((uint16_t)(p010leUV[i]) >> 2) |
    //                     ((uint16_t)(p010leUV[i + 1]) << 6); // 提取 10 位 u 分量

    //     uint16_t vVal = ((uint16_t)(p010leUV[i + 2]) >> 2) |
    //                     ((uint16_t)(p010leUV[i + 3]) << 6); // 提取 10 位 v 分量

    //     uData[idx_] = (uint8_t)(uVal);
    //     vData[idx_] = (uint8_t)(vVal);
    // }

    for (size_t i = 0, idx_ = 0; i < uvSize / 2; i += 2, idx_++) {
            uint16_t uVal = ((uint16_t)(p010leUV[i]) >> 2) |
                            ((uint16_t)(p010leUV[i + 1]) << 6); // 提取 10 位 u 分量
            int vIdx = i + uvSize / 2;
            uint16_t vVal = ((uint16_t)(p010leUV[vIdx]) >> 2) |
                            ((uint16_t)(p010leUV[vIdx + 1]) << 6); // 提取 10 位 v 分量
            uData[idx_] = (uint8_t)(uVal);
            vData[idx_] = (uint8_t)(vVal);
    }
}

// 将 8 位 YUV 数据转换为 RGBA
void convertYUVToRGBA(uint8_t* yData, uint8_t* uData, uint8_t* vData,
                      uint8_t* rgbaData, int width, int height)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int yIndex = y * width * 2 + x * 2;
            int uvIndex = (y * width + (x & ~1)) / 2; // Each pair of Y samples share one UV pair
            if (y == 0 && x <= 10 )
            std::cout << "uvIndex: " << uvIndex << std::endl;

            uint8_t yValue = yData[yIndex];
            uint8_t uValue = uData[uvIndex];
            uint8_t vValue = vData[uvIndex];

            // YUV 转 RGB
            int r = (int)(yValue + 1.402 * (vValue - 128));
            int g = (int)(yValue - 0.344136 * (uValue - 128) - 0.714136 * (vValue - 128));
            int b = (int)(yValue + 1.772 * (uValue - 128));

            // Clip RGB 值到 [0, 255] 范围
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);

            // 存储 RGBA 数据
            int rgbaIndex = (y * width + x) * 4;
            rgbaData[rgbaIndex]     = (uint8_t)r;   // R
            rgbaData[rgbaIndex + 1] = (uint8_t)g;   // G
            rgbaData[rgbaIndex + 2] = (uint8_t)b;   // B
            rgbaData[rgbaIndex + 3] = 255;           // A (完全不透明)
        }
    }
}

// 将 RGBA 数据保存为 BMP 文件
void saveRGBAtoBMP(const char* filename, uint8_t* rgbaData, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int rowSize = (width * 4 + 3) & ~3; // 每行的字节数，按 4 字节对齐
    int paddingSize = rowSize - (width * 4);
    int imageSize = rowSize * height;

    // BMP 文件头
    uint8_t fileHeader[14] = {
        'B', 'M',               // Signature
        (uint8_t)(imageSize + 54), (uint8_t)((imageSize + 54) >> 8),
        (uint8_t)((imageSize + 54) >> 16), (uint8_t)((imageSize + 54) >> 24), // File size
        0, 0,                   // Reserved
        0, 0,                   // Reserved
        54, 0, 0, 0             // Offset to pixel data
    };

    // BMP 信息头
    uint8_t infoHeader[40] = {
        40, 0, 0, 0,            // Header size
        (uint8_t)width, (uint8_t)(width >> 8), (uint8_t)(width >> 16), (uint8_t)(width >> 24), // Width
        (uint8_t)height, (uint8_t)(height >> 8), (uint8_t)(height >> 16), (uint8_t)(height >> 24), // Height
        1, 0,                   // Planes
        32, 0,                  // Bits per pixel
        0, 0, 0, 0,             // Compression
        (uint8_t)imageSize, (uint8_t)(imageSize >> 8), (uint8_t)(imageSize >> 16), (uint8_t)(imageSize >> 24), // Image size
        0, 0, 0, 0,             // X resolution
        0, 0, 0, 0,             // Y resolution
        0, 0,                   // Colors used
        0, 0                    // Important colors
    };

    fwrite(fileHeader, 1, 14, fp);
    fwrite(infoHeader, 1, 40, fp);

    // 写入像素数据（从底到顶）
    for (int y = height - 1; y >= 0; --y) {
        fwrite(rgbaData + y * width * 4, 1, width * 4, fp);
        if (paddingSize > 0) {
            uint8_t padding[3] = {0, 0, 0};
            fwrite(padding, 1, paddingSize, fp);
        }
    }

    fclose(fp);
}
