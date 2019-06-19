//
// Created by Tomer Shalev on 2019-06-16.
//

#pragma once

#include <math.h>
//template <typename T>

float lerp(float a, float b, float t);
float smooth_step(float t);
bool insideCircle(float x, float y, float centerX, float centerY, float radius);

//inline float edgeFunction(const Vec3 &a, const Vec3 &b, const Vec3 &c)
//{
//    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
//}

template <typename T>
inline T edgeFunction(T x0, T y0, T x1, T y1, T x2, T y2)
{
    return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
}


//template <>
//inline float edgeFunction(float x0, float y0, float x1, float y1, float x2, float y2);