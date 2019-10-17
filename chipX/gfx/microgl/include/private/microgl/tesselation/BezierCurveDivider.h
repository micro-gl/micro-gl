#pragma once

#include <microgl/vec2.h>
//#include <microgl/dynamic_array.h>
#include <microgl/dynamic_array.h>

using namespace microgl;

namespace tessellation {

    class BezierCurveDivider {
    public:
        using precision = unsigned char;

        enum class Type {
            Quadratic, Cubic
        };

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

        explicit BezierCurveDivider() = default;

        static void compute(const vec2_32i *points,
                            precision precision,
                            dynamic_array<vec2_32i> &output,
                            CurveDivisionAlgorithm algorithm,
                            Type $type);

    private:

        static void sub_divide_cubic_bezier(const vec2_32i *points, BezierCurveDivider::precision precision,
                                            dynamic_array<vec2_32i> &output,
                                            BezierCurveDivider::CurveDivisionAlgorithm algorithm);

        static void sub_divide_quadratic_bezier(const vec2_32i *points,
                                                precision precision,
                                                dynamic_array<vec2_32i> &output,
                                                CurveDivisionAlgorithm algorithm);


        static void uniform_sub_divide_cubic_bezier(const vec2_32i *points,
                                                    precision precision_point,
                                                    precision subdivision_bits,
                                                    dynamic_array<vec2_32i> &output
                                             );

        static void uniform_sub_divide_quadratic_bezier(const vec2_32i *points,
                                                        precision precision_point,
                                                        precision subdivision_bits,
                                                        dynamic_array<vec2_32i> &output
                                                 );

        static void adaptive_sub_divide_cubic_bezier_internal(const vec2_32i *points,
                                                              precision precision,
                                                              unsigned int tolerance_distance_pixels,
                                                              dynamic_array<vec2_32i> &output
                                                       );

        static void adaptive_sub_divide_cubic_bezier(const vec2_32i *points,
                                                     precision precision,
                                                     unsigned int tolerance_distance_pixels,
                                                     dynamic_array<vec2_32i> &output
                                              );

        static void adaptive_sub_divide_quadratic_bezier(const vec2_32i *points,
                                                         precision precision,
                                                         unsigned int tolerance_distance_pixels,
                                                         dynamic_array<vec2_32i> &output
                                                  );

        static vec2_32i lerp_fixed(int t, precision range_bits, const vec2_32i &a, const vec2_32i &b, precision point_precision);

        static void evaluate_quadratic_bezier_at(unsigned int t, precision range_bits,
                                                 const vec2_32i *points, precision sub_pixel_bits,
                                                 vec2_32i & output);

        static void evaluate_cubic_bezier_at(unsigned int t, precision range_bits,
                                             const vec2_32i *points, precision sub_pixel_bits,
                                             vec2_32i & output);

        static void split_cubic_bezier_at(unsigned int t,
                                          precision range_bits,
                                          const vec2_32i *points,
                                          precision sub_pixel_bits,
                                          vec2_32i & left_1, vec2_32i & left_2,
                                          vec2_32i & left_3, vec2_32i & left_4,
                                          vec2_32i & right_1, vec2_32i & right_2,
                                          vec2_32i & right_3, vec2_32i & right_4);

        static void split_quadratic_bezier_at(unsigned int t,
                                              precision range_bits,
                                              const vec2_32i *points,
                                              precision sub_pixel_bits,
                                              vec2_32i & left_1, vec2_32i & left_2, vec2_32i & left_3,
                                              vec2_32i & right_1, vec2_32i & right_2, vec2_32i & right_3);

        static void quadratic_to_cubic_bezier(const vec2_32i *points,
                                              vec2_32i &p0, vec2_32i &p1,
                                              vec2_32i &p2, vec2_32i &p3);

        static unsigned int compute_cubic_bezier_flatness(const vec2_32i *points, precision precision);
        static unsigned int compute_quadratic_bezier_flatness(const vec2_32i *points, precision precision);
        static bool is_cubic_bezier_flat(const vec2_32i *points, precision precision, unsigned int tolerance_distance_pixels);
        static bool is_quadratic_bezier_flat(const vec2_32i *points, precision precision, unsigned int threshold);

    };

}
