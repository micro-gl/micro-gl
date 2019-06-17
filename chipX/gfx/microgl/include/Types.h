#pragma once

#include <cstdint>

typedef struct {
    uint8_t r, g, b, a;
} color_t;

struct color_f_t {
    float r, g, b, a;

    color_f_t& operator*(const float & val)
    {
        r *= val;
        g *= val;
        b *= val;
        return *this;
    }

};

template<typename T>
struct vec2 {
    T x, y;
};

template<typename T>
struct vec3 {
    T x, y, z;
};

typedef vec2<float> vec2_f;
typedef vec2<uint8_t > vec2_8i;
typedef vec2<uint16_t > vec2_16i;
typedef vec2<uint32_t > vec2_32i;

typedef vec3<float> vec3_f;
typedef vec3<uint8_t > vec3_8i;
typedef vec3<uint16_t > vec3_16i;
typedef vec3<uint32_t > vec3_32i;