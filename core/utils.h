#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <cstdlib>
#include <cassert>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


void convert_hardware_yuv_to_rgba(const uint8_t *y_plane,
                                  const uint8_t *uv_plane,
                                  uint8_t *rgba_frame,
                                  int width,
                                  int height,
                                  AVPixelFormat hw_pix_fmt);

void convertP010LEToRGBA(uint8_t* yData, uint8_t* uvData, uint8_t* rgbaData,
                         int width, int height);

void convertP010LETo8bit(uint8_t* p010leY,  const size_t ySize, 
                         uint8_t* p010leUV, const size_t uvSize,
                         uint8_t* yData, uint8_t* uData, uint8_t* vData);

void convertYUVToRGBA(uint8_t* yData, uint8_t* uData, uint8_t* vData,
                      uint8_t* rgbaData, int width, int height);

#endif // UTILS_H
