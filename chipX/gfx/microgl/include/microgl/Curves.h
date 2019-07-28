//
// Created by Tomer Shalev on 2019-07-27.
//

#pragma once

#include <cstdint>
#include "Types.h"

namespace curves {

    vec2_32i lerp_fixed(const vec2_32i &a, const vec2_32i &b, int t, uint8_t range_bits);

    void split_cubic_bezier_at(unsigned int t, uint8_t range_bits,
                              const vec2_32i *points, uint8_t sub_pixel_bits,
                              vec2_32i & left_1, vec2_32i & left_2,
                              vec2_32i & left_3, vec2_32i & left_4,
                              vec2_32i & right_1, vec2_32i & right_2,
                              vec2_32i & right_3, vec2_32i & right_4);

    void split_quadratic_bezier_at(unsigned int t, uint8_t range_bits,
                                  const vec2_32i *points, uint8_t sub_pixel_bits,
                                  vec2_32i & left_1, vec2_32i & left_2, vec2_32i & left_3,
                                  vec2_32i & right_1, vec2_32i & right_2, vec2_32i & right_3);

    void evaluate_quadratic_bezier_at(unsigned int t, uint8_t range_bits,
                                     const vec2_32i *points, uint8_t sub_pixel_bits,
                                     vec2_32i & output);

    void evaluate_cubic_bezier_at(unsigned int t, uint8_t range_bits,
                                 const vec2_32i *points, uint8_t sub_pixel_bits,
                                 vec2_32i & output);


}