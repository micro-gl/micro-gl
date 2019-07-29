//
// Created by Tomer Shalev on 2019-07-27.
//

#pragma once

#include <cstdint>
#include <vector>
#include "Types.h"

namespace curves {

    enum class CurveDivisionAlgorithm {
        // highest quality for adaptive based on distance to real curve
        Adaptive_tolerance_distance_Small,
        // medium quality for adaptive based on distance to real curve
        Adaptive_tolerance_distance_Medium,
        // low quality for adaptive based on distance to real curve
        Adaptive_tolerance_distance_Large,
        // highest quality for uniform subdivision
        Uniform_64,
        // medium quality for uniform subdivision
        Uniform_32,
        // low quality for uniform subdivision
        Uniform_16,
    };

    vec2_32i lerp_fixed(int t, const vec2_32i &a, const vec2_32i &b, uint8_t precision, uint8_t range_bits);

    void sub_divide_cubic_bezier(const vec2_32i *points,
                                 uint8_t precision,
                                 std::vector<vec2_32i> &output,
                                 CurveDivisionAlgorithm algorithm);

    void sub_divide_quadratic_bezier(const vec2_32i *points,
                                     uint8_t precision,
                                     std::vector<vec2_32i> &output,
                                     CurveDivisionAlgorithm algorithm);

    // these can be hidden

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

    void evaluate_quadratic_bezier_at(const unsigned int t, const uint8_t range_bits,
                                      const vec2_32i *points, const uint8_t sub_pixel_bits,
                                      vec2_32i & output);

    void evaluate_cubic_bezier_at(const unsigned int t, const uint8_t range_bits,
                                  const vec2_32i *points, const uint8_t sub_pixel_bits,
                                  vec2_32i & output);

    void quadratic_to_cubic_bezier(const vec2_32i *points,
                                   vec2_32i &p0, vec2_32i &p1,
                                   vec2_32i &p2, vec2_32i &p3);

    unsigned int compute_cubic_bezier_flatness(const vec2_32i *points, uint8_t precision=0);
    unsigned int compute_quadratic_bezier_flatness(const vec2_32i *points, uint8_t precision=0);
    bool is_cubic_bezier_flat(const vec2_32i *points, uint8_t precision, unsigned int tolerance_distance_pixels);
    bool is_quadratic_bezier_flat(const vec2_32i *points, uint8_t precision, unsigned int tolerance_distance_pixels);

    void adaptive_sub_divide_cubic_bezier(const vec2_32i *points,
                                          uint8_t precision,
                                          unsigned int tolerance_distance_pixels,
                                          std::vector<vec2_32i> &output);

    void adaptive_sub_divide_quadratic_bezier(const vec2_32i *points,
                                              uint8_t precision,
                                              unsigned int tolerance_distance_pixels,
                                              std::vector<vec2_32i> &output);

    void uniform_sub_divide_cubic_bezier(const vec2_32i *points,
                                         uint8_t precision,
                                         uint8_t subdivision_bits,
                                         std::vector<vec2_32i> &output);

    void uniform_sub_divide_quadratic_bezier(const vec2_32i *points,
                                             uint8_t precision,
                                             uint8_t subdivision_bits,
                                             std::vector<vec2_32i> &output);
}