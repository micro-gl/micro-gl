//
// Created by Tomer Shalev on 2019-07-27.
//

#pragma once

#include <cstdint>
#include <vector>
#include "Types.h"

namespace curves {

    vec2_32i lerp_fixed(const vec2_32i &a, const vec2_32i &b, int t, uint8_t range_bits);

    void split_cubic_bezier_at(unsigned int t, uint8_t range_bits,
                               const vec2_32i *points,
                               uint8_t sub_pixel_bits,
                               vec2_32i & left_1, vec2_32i & left_2,
                               vec2_32i & left_3, vec2_32i & left_4,
                               vec2_32i & right_1, vec2_32i & right_2,
                               vec2_32i & right_3, vec2_32i & right_4);

    void split_quadratic_bezier_at(unsigned int t, uint8_t range_bits,
                                   const vec2_32i *points,
                                   uint8_t sub_pixel_bits,
                                   vec2_32i & left_1, vec2_32i & left_2, vec2_32i & left_3,
                                   vec2_32i & right_1, vec2_32i & right_2, vec2_32i & right_3);

    void evaluate_quadratic_bezier_at(unsigned int t, uint8_t range_bits,
                                     const vec2_32i *points, uint8_t sub_pixel_bits,
                                     vec2_32i & output);

    void evaluate_cubic_bezier_at(unsigned int t, uint8_t range_bits,
                                 const vec2_32i *points, uint8_t sub_pixel_bits,
                                 vec2_32i & output);

    void quadratic_to_cubic_bezier(const vec2_32i *points,
                                   vec2_32i &p0, vec2_32i &p1,
                                   vec2_32i &p2, vec2_32i &p3);

    unsigned int compute_cubic_bezier_flatness(const vec2_32i *points, uint8_t precision=0);
    unsigned int compute_quadratic_bezier_flatness(const vec2_32i *points, uint8_t precision=0);
    bool is_cubic_bezier_flat(const vec2_32i *points, uint8_t precision, unsigned int threshold);
    bool is_quadratic_bezier_flat(const vec2_32i *points, uint8_t precision, unsigned int threshold);

    void adaptive_sub_divide_cubic_bezier(const vec2_32i *points,
                                          uint8_t precision,
                                          unsigned int threshold,
                                          std::vector<vec2_32i> &output);

    void adaptive_sub_divide_quadratic_bezier(const vec2_32i *points,
                                              uint8_t precision,
                                              unsigned int threshold,
                                              std::vector<vec2_32i> &output);

}