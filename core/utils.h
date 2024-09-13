#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <cstdlib>
#include <cassert>
#include <time.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
}


void convert_hardware_yuv_to_rgba(const uint8_t *y_plane,
                                  const uint8_t *uv_plane,
                                  uint8_t *rgba_frame,
                                  int width,
                                  int height,
                                  AVPixelFormat hw_pix_fmt);

void convert_nv12_to_rgba(const uint8_t *y_plane,
                          const uint8_t *uv_plane,
                          uint8_t *rgba_frame,
                          int width,
                          int height);


uint64_t C416TimeConvert(uint64_t t);

uint64_t DJIVideoCreateTimeConvert(const char* s);

uint64_t TimeBase2Timestamp(uint64_t pts, uint64_t createTime, AVRational timeBase);

#endif // UTILS_H
