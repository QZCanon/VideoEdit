#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <cstdlib>
#include <cassert>

void convertP010LEToRGBA(uint8_t* yData, uint8_t* uvData, uint8_t* rgbaData,
                         int width, int height);

void convertP010LETo8bit(uint8_t* p010leY,  const size_t ySize, 
                         uint8_t* p010leUV, const size_t uvSize,
                         uint8_t* yData, uint8_t* uData, uint8_t* vData);

void convertYUVToRGBA(uint8_t* yData, uint8_t* uData, uint8_t* vData,
                      uint8_t* rgbaData, int width, int height);

#endif // UTILS_H
