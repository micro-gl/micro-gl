//
// Created by Tomer Shalev on 2019-06-16.
//

#pragma once

#include <math.h>
#include <stdint.h>

template <typename T>
inline T edgeFunction(T x0, T y0, T x1, T y1, T x2, T y2)
{
    return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
}

inline float lerp(float a, float b, float t) {
    return (a * (1.0 - t)) + (b * t);
}

inline float smooth_step(float t) {
    return t*t*(3 - t*2);
}

inline bool insideCircle(float x, float y, float centerX, float centerY, float radius) {
    float distX = x - (centerX), distY = y - (centerY);
    float distance = sqrt(distX * distX + distY * distY);
    return (distance <= radius);

}

inline uint32_t alpha_max_plus_beta_min(uint32_t x, uint32_t y) {
    return (x>y) ? (x + (y>>1)) : (y + (x>>1));
}

uint32_t sqrt_int(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1 << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    int index=0;
    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res + ( one<<1);
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

inline int sdCircle( float px, float py, unsigned int r )
{
    return sqrt(px*px + py*py) - r;
}

inline int sdCircle( int px, int py, int cx, int cy, unsigned int r )
{
    int dx = (px-cx), dy = py-cy;
    return sqrt_int(dx*dx + dy*dy) - r;
}

inline float smoothstep(float edge0, float edge1, float x) {
    float t = fmin((x - edge0) / (edge1 - edge0), 1.0);
    t = fmax(t, 0.0);
    return t * t * (3.0 - 2.0 * t);
}
