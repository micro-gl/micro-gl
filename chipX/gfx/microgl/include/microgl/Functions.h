//
// Created by Tomer Shalev on 2019-06-16.
//

#pragma once

#include <math.h>

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
