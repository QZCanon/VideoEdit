#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void convertP010LEToRGBA(uint8_t* yData, uint8_t* uvData, uint8_t* rgbaData,
                         int width, int height)
{
    int uvWidth = width / 2;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int yIndex = y * width * 2 + x * 2; // 2 bytes per Y value
            int uvIndex = (y * uvWidth + (x / 2)) * 4; // 2 bytes per U and V

            uint16_t yValue = (yData[yIndex] | (yData[yIndex + 1] << 8)) >> 6; // 10 bits Y
            uint16_t uValue = (uvData[uvIndex] | (uvData[uvIndex + 1] << 8)) >> 6; // 10 bits U
            uint16_t vValue = (uvData[uvIndex + 2] | (uvData[uvIndex + 3] << 8)) >> 6; // 10 bits V

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

int main() {
    int width = 640;
    int height = 480;

    // 分配 YUV 数据和 RGBA 数据内存
    uint8_t *yData = (uint8_t*)malloc(width * height * 2);
    uint8_t *uvData = (uint8_t*)malloc((width / 2) * height * 4); // U and V interleaved
    uint8_t *rgbaData = (uint8_t*)malloc(width * height * 4);

    // 填充 YUV 数据的示例代码...

    // 转换
    convertP010LEToRGBA(yData, uvData, rgbaData, width, height);

    // 使用 rgbaData...

    // 释放内存
    free(yData);
    free(uvData);
    free(rgbaData);

    return 0;
}
