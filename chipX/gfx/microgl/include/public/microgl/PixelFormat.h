#pragma once

#include <cstdint>
#include <math.h>
#include <microgl/vec2.h>
#include <microgl/color.h>

using namespace microgl;

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

uint8_t countPixelFormatBytes(const PixelFormat & format);

inline uint32_t encode_RGBA8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (r << 24) + (g << 16) + (b << 8) + a;
}

inline uint32_t encode_ARGB8888(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (a << 24) + (r << 16) + (g << 8) + b;
}

inline uint32_t encode_RGBA5551(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((r & 0x1F) << 11) + ((g & 0x1F) << 6) + ((b & 0x1F) << 1) + (a & 0x1);
}

inline uint32_t encode_ARGB1555(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((a & 0x1) << 15) + ((r & 0x1F) << 10) + ((g & 0x1F) << 5) + (b & 0x1F);
}

inline uint32_t encode_RGBA4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((r & 0b00001111) << 12) + ((g & 0b00001111) << 8) + ((b & 0b00001111) << 4) + (a & 0b00001111);
}

inline uint32_t encode_ARGB4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((a & 0b00001111) << 12) + ((r & 0b00001111) << 8) + ((g & 0b00001111) << 4) + (b & 0b00001111);
}

inline uint32_t encode_RGB888(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) + (g << 8) + b;
}

inline uint32_t encode_RGB888_f(color_f_t &col) {
    return (uint8_t(col.r*255) << 16) + (uint8_t(col.g*255) << 8) + uint8_t(col.b*255);
}

inline uint16_t encode_RGB565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0x1F) << 11) + ((g & 0x3F) << 5) + ((b & 0x1F));
}

inline uint16_t encode_RGB555(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0b00011111) << 10) + ((g & 0b00011111) << 5) + ((b & 0b00011111));
}

inline uint16_t encode_RGB565(color_t rgb) {
    return ((rgb.r & 0b00011111) << 11) + ((rgb.g & 0b00111111) << 5) + ((rgb.b & 0b00011111));
}

inline uint8_t encode_RGB332(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0b00000111) << 5) + ((g & 0b00000111) << 2) + ((b & 0b00000011));
}

inline uint32_t encodeFloatRGB(float r, float g, float b, float a, PixelFormat format) {
    uint32_t res = 0;

    switch (format) {

        case RGBA8888:
            res = encode_RGBA8888(255.0*r, 255.0*g, 255.0*b, 255.0*a);
            break;
        case ARGB8888:
            res = encode_ARGB8888(255.0*r, 255.0*g, 255.0*b, 255.0*a);
            break;
        case RGBA5551:
            res = encode_RGBA5551(31*r, 31*g, 31*b, int8_t(a));
            break;
        case ARGB1555:
            res = encode_ARGB1555(31*r, 31*g, 31*b, a);
            break;
        case RGBA4444:
            res = encode_RGBA4444(15*r, 15*g, 15*b, 15*a);
            break;
        case ARGB4444:
            res = encode_ARGB4444(15*r, 15*g, 15*b, 15*a);
            break;
        case RGB888:
            res = encode_RGB888(255.0*r, 255.0*g, 255.0*b);
            break;
        case RGB565:
            res = encode_RGB565(31*r, 63*g, 31*b);
            break;
        case RGB555:
            res = encode_RGB555(31*r, 31*g, 31*b);
            break;
        case RGB332:
            res = encode_RGB332(7*r, 7*g, 3*b);
            break;
        case RGB8:
            res = 255.0*r;
            break;
    }

    return res;
}

inline uint32_t encodeFloatRGB(color_f_t color, PixelFormat format) {
    return encodeFloatRGB(color.r, color.g, color.b, color.a, format);
}

inline uint32_t encodeRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a, PixelFormat format) {
    uint32_t res = 0;

    switch (format) {

        case RGBA8888:
            res = encode_RGBA8888(r, g, b, a);
            break;
        case ARGB8888:
            res = encode_ARGB8888(r, g, b, a);
            break;
        case RGBA5551:
            res = encode_RGBA5551(r, g, b, a);
            break;
        case ARGB1555:
            res = encode_ARGB1555(r, g, b, a);
            break;
        case RGBA4444:
            res = encode_RGBA4444(r, g, b, a);
            break;
        case ARGB4444:
            res = encode_ARGB4444(r, g, b, a);
            break;
        case RGB888:
            res = encode_RGB888(r, g, b);
            break;
        case RGB565:
            res = encode_RGB565(r, g, b);
            break;
        case RGB555:
            res = encode_RGB555(r, g, b);
            break;
        case RGB332:
            res = encode_RGB332(r, g, b);
            break;
        case RGB8:
            res = r;
            break;
    }

    return res;
}

inline uint32_t encodeRGB(color_t color, PixelFormat format) {
    return encodeRGB(color.r, color.g, color.b, color.a, format);
}

inline color_t decodeRGB(int color, PixelFormat format) {

    color_t res{};

    res.a = 1;

    switch (format) {
        case RGBA8888:
            res.r = (color & 0xFF000000) >> 24;
            res.g = (color & 0x00FF0000) >> 16;
            res.b = (color & 0x0000FF00) >> 8;
            res.a = (color & 0x000000FF);

            break;
        case ARGB8888:
            res.a = (color & 0xFF000000) >> 24;
            res.r = (color & 0x00FF0000) >> 16;
            res.g = (color & 0x0000FF00) >> 8;
            res.b = (color & 0x000000FF);

            break;
        case RGBA5551:
            res.r = (color & 0b1111100000000000) >> 11;
            res.g = (color & 0b0000011111000000) >> 6;
            res.b = (color & 0b0000000000111110) >> 1;
            res.a = (color & 0b0000000000000001);

            break;

        case ARGB1555:
            res.a = (color & 0b1000000000000000) >> 15;
            res.r = (color & 0b0111110000000000) >> 10;
            res.g = (color & 0b0000001111100000) >> 5;
            res.b = (color & 0b0000000000011111);

            break;

        case RGBA4444:
            res.r = (color & 0b1111000000000000) >> 12;
            res.g = (color & 0b0000111100000000) >> 8;
            res.b = (color & 0b0000000011110000) >> 4;
            res.a = (color & 0b0000000000001111);

            break;

        case ARGB4444:
            res.a = (color & 0b1111000000000000) >> 12;
            res.r = (color & 0b0000111100000000) >> 8;
            res.g = (color & 0b0000000011110000) >> 4;
            res.b = (color & 0b0000000000001111);

            break;

        case RGB888:
            res.r = (color & 0xFF0000) >> 16;
            res.g = (color & 0x00FF00) >> 8;
            res.b = (color & 0x0000FF);
            break;
        case RGB565:
            res.r = (color & 0xF800) >> 11;         // rrrrr... ........ -> 000rrrrr
            res.g = (color & 0x07E0) >> 5;          // .....ggg ggg..... -> 00gggggg
            res.b = (color & 0x1F);                 // ............bbbbb -> 000bbbbb
            break;
        case RGB555:
            res.r = (color & 0b0111110000000000) >> 10;
            res.g = (color & 0b0000001111100000) >> 5;
            res.b = (color & 0b0000000000011111);
            break;
        case RGB332:
            res.r = (color & 0b11100000) >> 5;
            res.g = (color & 0b00011100) >> 2;
            res.b = (color & 0b00000011);
            break;
        case RGB8:
            res.r = res.g = res.b = color;

            break;
    }

    return res;
}

inline color_f_t decodeFloatRGB(int color, PixelFormat format) {
    uint8_t r_max_value, g_max_value, b_max_value, a_max_value;
    color_t col = decodeRGB(color, format);
    color_f_t res{};

    switch (format) {
        case RGBA8888:
            r_max_value = 255; g_max_value = 255; b_max_value = 255; a_max_value = 255;
            break;
        case ARGB8888:
            r_max_value = 255; g_max_value = 255; b_max_value = 255; a_max_value = 255;
            break;
        case RGBA5551:
            r_max_value = 31; g_max_value = 31; b_max_value = 31; a_max_value = 2;
            break;
        case ARGB1555:
            r_max_value = 31; g_max_value = 31; b_max_value = 31; a_max_value = 2;
            break;
        case RGBA4444:
            r_max_value = 15; g_max_value = 15; b_max_value = 15; a_max_value = 15;
            break;
        case ARGB4444:
            r_max_value = 15; g_max_value = 15; b_max_value = 15; a_max_value = 15;
            break;
        case RGB888:
            r_max_value = 255; g_max_value = 255; b_max_value = 255; a_max_value = 255;
            break;
        case RGB565:
            r_max_value = 31; g_max_value = 63; b_max_value = 31; a_max_value = col.a;
            break;
        case RGB555:
            r_max_value = 31; g_max_value = 31; b_max_value = 31; a_max_value = col.a;
            break;
        case RGB332:
            r_max_value = 7; g_max_value = 7; b_max_value = 3; a_max_value = col.a;
            break;
        case RGB8:
            r_max_value = 255; g_max_value = 255; b_max_value = 255; a_max_value = 255;
            break;
        default:
            r_max_value = 255; g_max_value = 255; b_max_value = 255; a_max_value = 255;
    }

    res.r = (float)col.r / r_max_value;
    res.g = (float)col.g / g_max_value;
    res.b = (float)col.b / b_max_value;
    res.a = (float)col.a / a_max_value;

    return res;
}

/*
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
 */
