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


/*
 * fixed sqrtF2F( fixed v );
 *
 * Compute fixed to fixed square root
 * RETURNS the fixed point square root of v (fixed)
 * REQUIRES v is positive
 */
uint32_t sqrtF2F ( uint32_t x )
{
    uint32_t t, q, b, r;
    r = x;
    b = 0x40000000;
    q = 0;
    while( b > 0x40 )
    {
        t = q + b;
        if( r >= t )
        {
            r -= t;
            q = t + b; // equivalent to q += 2*b
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 8;
    return q;
}

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
int32_t sqrtI2F( int32_t v )
{
    uint32_t t, q, b, r;
    if( !v ) return 0;
    r = v;
    b = 0x40000000;
    q = 0;

    while( b > 0 )
    {
        t = q + b;
        if( r >= t )
        {
            r -= t;
            q = t + b; // equivalent to q += 2*b
        }
        r <<= 1;
        b >>= 1;
    }

    if( r >= q ) ++q;

    return q;
}

int32_t sqrtI2I( int32_t v )
{
    uint32_t t, q, b, r;
    r = v;           // r = v - x²
    b = 0x40000000;  // a²
    q = 0;           // 2ax


    while( b > 0 )
    {
        t = q + b;   // t = 2ax + a²
        q >>= 1;     // if a' = a/2, then q' = q/2
        if( r >= t ) // if (v - x²) >= 2ax + a²
        {
            r -= t;  // r' = (v - x²) - (2ax + a²)
            q += b;  // if x' = (x + a) then ax' = ax + a², thus q' = q' + b
        }
        b >>= 2;     // if a' = a/2, then b' = b / 4
    }
    return q;
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

/////

//https://stackoverflow.com/questions/31117497/fastest-integer-square-root-in-the-least-amount-of-instructions

static const uint8_t debruijn32[32] = {
        15,  0, 11, 0, 14, 11, 9, 1, 14, 13, 12, 5, 9, 3, 1, 6,
        15, 10, 13, 8, 12,  4, 3, 5, 10,  8,  4, 2, 7, 2, 7, 6
};

/* based on CLZ emulation for non-zero arguments, from
 * http://stackoverflow.com/questions/23856596/counting-leading-zeros-in-a-32-bit-unsigned-integer-with-best-algorithm-in-c-pro
 */
inline uint8_t shift_for_msb_of_sqrt(uint32_t x) {
    x |= x >>  1;
    x |= x >>  2;
    x |= x >>  4;
    x |= x >>  8;
    x |= x >> 16;
    x++;
    return debruijn32 [x * 0x076be629 >> 27];
}

inline uint32_t sqrt_int_2(uint32_t n) {
    if (n==0) return 0;

    uint32_t s = shift_for_msb_of_sqrt(n);
    uint32_t c = 1 << s;
    uint32_t g = c;

    switch (s) {
        case 9:
        case 5:
            if (g*g > n) {
                g ^= c;
            }
            c >>= 1;
            g |= c;
        case 15:
        case 14:
        case 13:
        case 8:
        case 7:
        case 4:
            if (g*g > n) {
                g ^= c;
            }
            c >>= 1;
            g |= c;
        case 12:
        case 11:
        case 10:
        case 6:
        case 3:
            if (g*g > n) {
                g ^= c;
            }
            c >>= 1;
            g |= c;
        case 2:
            if (g*g > n) {
                g ^= c;
            }
            c >>= 1;
            g |= c;
        case 1:
            if (g*g > n) {
                g ^= c;
            }
            c >>= 1;
            g |= c;
        case 0:
            if (g*g > n) {
                g ^= c;
            }
    }

    /* now apply one or two rounds of Newton's method */
    switch (s) {
        case 15:
        case 14:
        case 13:
        case 12:
        case 11:
        case 10:
            g = (g + n/g) >> 1;
        case 9:
        case 8:
        case 7:
        case 6:
            g = (g + n/g) >> 1;
    }

    /* correct potential error at m^2-1 for Newton's method */
    return (g==65536 || g*g>n) ? g-1 : g;
}

////



inline float sdCircle_f( int px, int py, int cx, int cy, unsigned int r )
{
    int dx = (px-cx), dy = py-cy;
    return sqrt(dx*dx + dy*dy) - r;
}

inline int sdCircle_int( int px, int py, int cx, int cy, unsigned int r )
{
    int dx = (px-cx), dy = py-cy;
    return sqrt_int(dx*dx + dy*dy) - (r);
}

/**
 * perform distace to a circle, input is integer, output is a fixed point
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
inline int sdCircle_fixed( int px, int py, int cx, int cy, unsigned int r )
{
    int dx = (px-cx), dy = py-cy;
    return sqrtI2F((dx*dx + dy*dy)) - (r<<16);
}

inline float smoothstep(float edge0, float edge1, float x) {
    float t = fmin((x - edge0) / (edge1 - edge0), 1.0);
    t = fmax(t, 0.0);
    return t * t * (3.0 - 2.0 * t);
}

