#pragma once

#include <cmath>
#include <cstdint>
#include <microgl/vec2.h>
#include "Fixed.h"

using namespace microgl;

namespace functions {

#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))
#define min3(a, b, c) (min_(min_(a,b), c))
#define max3(a, b, c) (max_(max_(a,b), c))
#define clamp_(v, e0, e1) (min_(max_(v,e0),e1))
#define abs(a) ((a)<0 ? -(a) : (a))

    template <typename T>
    inline int min(const T &p0, const T &p1) {
        return p0<p1 ? p0 : p1;
    }

    template <typename T>
    inline int min(const T &p0, const T &p1, const T &p2) {
        return min(min(p0, p1), p2);
    }

    template <typename T>
    inline int min(const T &p0, const T &p1, const T &p2, const T &p3) {
        return min(min(min(p0, p1), p2), p3);
    }

    template <typename T>
    inline int max(const T &p0, const T &p1) {
        return p0>p1 ? p0 : p1;
    }

    template <typename T>
    inline int max(const T &p0, const T &p1, const T &p2) {
        return max(max(p0, p1), p2);
    }

    template <typename T>
    inline int max(const T &p0, const T &p1, const T &p2, const T &p3) {
        return max(max(max(p0, p1), p2), p3);
    }

    template <typename T>
    inline int clamp(const T &val, const T &e0, const T &e1) {
        return min(max(val,e0),e1);
    }

    uint32_t sqrt_int(uint32_t a_nInput);

    template <typename T>
    inline T edgeFunction(T x0, T y0, T x1, T y1, T x2, T y2)
    {
        return (x2 - x0) * (y1 - y0) - (y2 - y0) * (x1 - x0);
    }

    template<typename T>
    inline vec2<T> intersect_lines(const vec2<T>& p1, const vec2<T>& p2,
                                   const vec2<T>& q1, const vec2<T>& q2)
    {
        vec2<T> result;
        T denominator = (p1.x - p2.x)*(q1.y - q2.y) - (p1.y-p2.y) * (q1.x-q2.x);
        T a1 = (p1.x*p2.y - p1.y*p2.x), a2 = (q1.x*q2.y - q1.y*q2.x);


        result.x = a1*(q1.x - q2.x) - (p1.x - p2.x) * a2;
        result.x /= denominator;

        result.y = a1*(q1.y - q2.y) - (p1.y - p2.y) * a2;
        result.y /= denominator;

        return result;
    }

    float lerp(float a, float b, float t);
    float smooth_step(float t);
    bool insideCircle(float x, float y, float centerX, float centerY, float radius);
    unsigned int length(const vec2_32i& a, const vec2_32i& b);
    unsigned int length(const vec2_32i& a, const vec2_32i& b, uint8_t precision);
    bool isParallelogram(const vec2_32i& p0, const vec2_32i& p1,
                                const vec2_32i& p2, const vec2_32i& p3,
                                int precisionRadius=2);
    bool isAxisAlignedRectangle(const vec2_32i& p0, const vec2_32i& p1,
                                       const vec2_32i& p2, const vec2_32i& p3);
    bool isRectangle(const vec2_32i& p0, const vec2_32i& p1,
                            const vec2_32i& p2, const vec2_32i& p3);
    typedef vec2<fixed_signed> vec2_fixed_signed;
    // this equals twice the signed triangle area - the parallelogram
    int orient2d(const vec2_fixed_signed& a, const vec2_fixed_signed& b,
                        const vec2_fixed_signed& c, uint8_t precision);
    // this equals twice the signed triangle area - the parallelogram
    int orient2d(const vec2_32i& a, const vec2_32i& b, const vec2_32i& c);


    /*
     * fixed sqrtF2F( fixed v );
     *
     * Compute fixed to fixed square root
     * RETURNS the fixed point square root of v (fixed)
     * REQUIRES v is positive
     */
    uint32_t sqrtF2F ( uint32_t x );

    /*
     * fixed sqrtI2F( long v );
     *
     * Compute int32_t to fixed square root
     * RETURNS the fixed point square root of v
     * REQUIRES v is positive
     *          The less significant bit may be inaccurate for some values bigger
     *          than 57 millions because of computation overflow and rounding error
     *
     * Shifts comma one bit left at each iteration. Last instruction adjusts
     * rounding approximation in last bit.
     */
    int32_t sqrtI2F( int32_t v );

    int32_t sqrtI2I( int32_t v );


    int signed_distance_circle_raised_quad( int px, int py, int cx, int cy, unsigned int r );

    int signed_distance_circle_raised_quad( fixed_signed px, fixed_signed py,
                                                   fixed_signed cx, fixed_signed cy,
                                                   fixed_signed r, uint8_t p);

    float sdCircle_float(int px, int py, int cx, int cy, unsigned int r);

    int sdCircle_int( int px, int py, int cx, int cy, unsigned int r );

    /**
     * perform distance to a circle, input is integer, output is a fixed point
     * integer for precision
     *
     * @param px point x
     * @param py point y
     * @param cx center x of circle
     * @param cy center y of circle
     * @param r radius of circle
     *
     * @return  a fixed point distance
     */
    int sdCircle_fixed( int px, int py, int cx, int cy, unsigned int r );

    float smoothstep(float edge0, float edge1, float x);

}
