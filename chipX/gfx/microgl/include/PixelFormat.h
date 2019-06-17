#pragma once

#include <cstdint>
#include <math.h>
#include "Types.h"

enum PixelFormat {
    RGBA8888,
    ARGB8888,
    RGBA4444,
    ARGB4444,
    RGBA5551,
    ARGB1555,
    RGB888,
    RGB565,
    RGB332,
    RGB555,
    RGB8,
};

uint32_t encode_RGBA8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t encode_ARGB8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t encode_ARGB1555(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t encode_RGBA5551(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t encode_RGBA4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t encode_ARGB4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t encode_RGB888(uint8_t r, uint8_t g, uint8_t b);
uint16_t encode_RGB565(uint8_t r, uint8_t g, uint8_t b);
uint16_t encode_RGB555(uint8_t r, uint8_t g, uint8_t b);
uint8_t encode_RGB332(uint8_t r, uint8_t g, uint8_t b);

uint32_t encodeRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a, PixelFormat format);
uint32_t encodeRGB(color_t color, PixelFormat format);
uint32_t encodeFloatRGB(color_f_t color, PixelFormat format);
uint32_t encodeFloatRGB(float r, float g, float b, float a, PixelFormat format);
color_t decodeRGB(int color, PixelFormat format);
color_f_t decodeFloatRGB(int color, PixelFormat format);