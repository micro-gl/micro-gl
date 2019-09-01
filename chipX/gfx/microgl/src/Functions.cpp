//
// Created by Tomer Shalev on 2019-06-16.
//

#include <microgl/Functions.h>

namespace functions {

    float smooth_step(float t) {
        return t * t * (3 - t * 2);
    }

    float lerp(float a, float b, float t) {
        return (a * (1.0 - t)) + (b * t);
    }

    bool insideCircle(float x, float y, float centerX, float centerY, float radius) {
        float distX = x - (centerX), distY = y - (centerY);
        float distance = sqrt(distX * distX + distY * distY);
        return (distance <= radius);

    }

    unsigned int length(const vec2_32i &a, const vec2_32i &b) {
        int dx = (a.x-b.x), dy = (a.y-b.y);
        return sqrt_int(dx*dx + dy*dy);
    }

    unsigned int length(const vec2_32i &a, const vec2_32i &b, uint8_t precision) {
        int dx = (a.x-b.x), dy = (a.y-b.y);
        // shift right since sqrt is compressing
//        return sqrt_int(fixed_mul_fixed_2(dx, dx, precision) + fixed_mul_fixed_2(dy,dy, precision));//<<(precision>>1);
//        return sqrt_64(fixed_mul_fixed_2(dx, dx, precision) + fixed_mul_fixed_2(dy,dy, precision));//<<(precision>>1);
        return sqrt_64((uint64_t)dx*dx + (uint64_t)dy*dy);
    }

    bool
    isParallelogram(const vec2_32i &p0, const vec2_32i &p1, const vec2_32i &p2, const vec2_32i &p3, int precisionRadius) {
        auto e0 = p1 - p0, e2= p3-p2;
        auto e1 = p2 - p1, e3= p0-p3;

        int a1 = abs((int)(length(p1, p0) - length(p3, p2)));
        int a2 = abs((int)(length(p2, p1) - length(p0, p3)));

        return a1<=precisionRadius && a2<=precisionRadius;
    }

    bool isAxisAlignedRectangle(const vec2_32i &p0, const vec2_32i &p1, const vec2_32i &p2, const vec2_32i &p3) {

        return p0.x==p3.x && p1.x==p2.x && p0.y==p1.y && p3.y==p2.y;
    }

    bool isRectangle(const vec2_32i &p0, const vec2_32i &p1, const vec2_32i &p2, const vec2_32i &p3) {

        auto e0 = p1 - p0, e2= p3-p2;
        auto e1 = p2 - p1, e3= p0-p3;

        int dp_1 = e0*e1;
        int dp_2 = e1*e2;
        int dp_3 = e2*e3;

        return ((dp_1 | dp_2 | dp_3)==0);
    }

    int orient2d(const vec2_fixed_signed &a, const vec2_fixed_signed &b, const vec2_fixed_signed &c, uint8_t precision) {
        return fixed_mul_fixed_2(b.x-a.x, c.y-a.y, precision) -
               fixed_mul_fixed_2(b.y-a.y, c.x-a.x, precision);
    }

    int orient2d(const vec2_32i &a, const vec2_32i &b, const vec2_32i &c) {
        return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    }

    uint32_t sqrtF2F(uint32_t x) {
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

    int32_t sqrtI2F(int32_t v) {
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

    int32_t sqrtI2I(int32_t v) {
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

    uint32_t sqrt_int(uint32_t a_nInput) {
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

    uint32_t sqrt_64(uint64_t a_nInput) {
        uint64_t op  = a_nInput;
        uint64_t res = 0;
        uint64_t one = uint64_t(1) << 62; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


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

    int signed_distance_circle_raised_quad(int px, int py, int cx, int cy, unsigned int r) {
        int dx = (px-cx), dy = py-cy;
        return (((dx*dx + dy*dy)) - ((r*r)));
    }

    int signed_distance_circle_raised_quad(fixed_signed px, fixed_signed py, fixed_signed cx, fixed_signed cy,
                                           fixed_signed r, uint8_t p) {
        fixed_signed dx = (px-cx), dy = py-cy;

        return (fixed_mul_fixed_2(dx, dx, p) + fixed_mul_fixed_2(dy, dy, p) - fixed_mul_fixed_2(r, r, p));
    }

    float sdCircle_float(int px, int py, int cx, int cy, unsigned int r) {
        int dx = (px-cx), dy = py-cy;
        return sqrt(dx*dx + dy*dy) - r;
    }

    int sdCircle_int(int px, int py, int cx, int cy, unsigned int r) {
        int dx = (px-cx), dy = py-cy;
        return sqrt_int(dx*dx + dy*dy) - (r);
    }

    int sdCircle_fixed(int px, int py, int cx, int cy, unsigned int r) {
        int dx = (px-cx), dy = py-cy;
        return sqrtI2F((dx*dx + dy*dy)) - (r<<16);
    }

    float smoothstep(float edge0, float edge1, float x) {
        float t = fmin((x - edge0) / (edge1 - edge0), 1.0);
        t = fmax(t, 0.0);
        return t * t * (3.0 - 2.0 * t);
    }

}