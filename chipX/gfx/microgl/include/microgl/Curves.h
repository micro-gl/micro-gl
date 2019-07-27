//
// Created by Tomer Shalev on 2019-07-27.
//

#pragma once

#include <cstdint>
#include "Types.h"

namespace curves {

    inline void evaluate_quadratic_bezier_at(const unsigned int t, const uint8_t range_bits,
                                             const vec2_32i *points, const uint8_t sub_pixel_bits,
                                             vec2_32i & output) {

        unsigned int resolution = range_bits;
        unsigned int resolution_double = resolution<<1;
        unsigned int N_SEG = (1 << resolution); // 64 resolution

        unsigned int comp = N_SEG - t;
        unsigned int a = comp * comp;
        unsigned int b = (t * comp) << 1;
        unsigned int c = t * t;
        output.x = (a * points[0].x + b * points[1].x + c * points[2].x) >> resolution_double;
        output.y = (a * points[0].y + b * points[1].y + c * points[2].y) >> resolution_double;
    }

    inline void evaluate_cubic_bezier_at(const unsigned int t, const uint8_t range_bits,
                                         const vec2_32i *points, const uint8_t sub_pixel_bits,
                                         vec2_32i & output) {

        unsigned int resolution = range_bits;
        unsigned int resolution_triple = resolution*3;
        unsigned int N_SEG = (1 << resolution); // 64 resolution

        // (n-t)^2 => n*n, t*t, n*t
        // (n-t)^3 => n*n*n, t*t*n, n*n*t, t*t*t
        //10
        // todo: we can use a LUT if using more point batches
        unsigned int comp = N_SEG - t;
        unsigned int comp_times_comp = comp * comp;
        unsigned int t_times_t = t * t;
        unsigned int a = comp * comp_times_comp;
        unsigned int b = 3 * (t * comp_times_comp);
        unsigned int c = 3*t_times_t*comp;
        unsigned int d = t*t_times_t;

        output.x = ((long)a * points[0].x + (long)b * points[1].x + (long)c * points[2].x + (long)d * points[3].x)>>resolution_triple;
        output.y = ((long)a * points[0].y + (long)b * points[1].y + (long)c * points[2].y + (long)d * points[3].y)>>resolution_triple;
    }


}