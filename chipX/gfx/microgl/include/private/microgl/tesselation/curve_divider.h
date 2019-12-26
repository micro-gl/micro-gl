#pragma once

#include <microgl/vec2.h>
#include <microgl/dynamic_array.h>

using namespace microgl;

namespace tessellation {

    template <typename number>
    class curve_divider {
    public:
        using precision = unsigned char;
        using vertex = vec2<number>;
        using output = dynamic_array<vertex>;
        using index = unsigned int;

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

        explicit curve_divider() = default;

        static void compute(const vertex *points,
                            precision precision,
                            output &output,
                            CurveDivisionAlgorithm algorithm,
                            Type $type);

    private:

        static void sub_divide_cubic_bezier(const vertex *points, curve_divider::precision precision,
                                            output &output,
                                            curve_divider::CurveDivisionAlgorithm algorithm);

        static void sub_divide_quadratic_bezier(const vertex *points,
                                                precision precision,
                                                output &output,
                                                CurveDivisionAlgorithm algorithm);


        static void uniform_sub_divide_cubic_bezier(const vertex *points,
                                                    precision precision_point,
                                                    precision subdivision_bits,
                                                    output &output
                                             );

        static void uniform_sub_divide_quadratic_bezier(const vertex *points,
                                                        output &output
                                                 );

        static void adaptive_sub_divide_cubic_bezier_internal(const vertex *points,
                                                              precision precision,
                                                              unsigned int tolerance_distance_pixels,
                                                              output &output
                                                       );

        static void adaptive_sub_divide_cubic_bezier(const vertex *points,
                                                     precision precision,
                                                     unsigned int tolerance_distance_pixels,
                                                     output &output
                                              );

        static void adaptive_sub_divide_quadratic_bezier(const vertex *points,
                                                         precision precision,
                                                         unsigned int tolerance_distance_pixels,
                                                         output &output
                                                  );

        static vertex lerp_fixed(const number t, const vertex &a, const vertex &b);

        static void evaluate_quadratic_bezier_directly_at(const number t,
                                                 const vertex *points,
                                                 vertex & output);

        static void evaluate_cubic_bezier_at(unsigned int t, precision range_bits,
                                             const vertex *points, precision sub_pixel_bits,
                                             vertex & output);

        static void split_cubic_bezier_at(unsigned int t,
                                          precision range_bits,
                                          const vertex *points,
                                          precision sub_pixel_bits,
                                          vertex & left_1, vertex & left_2,
                                          vertex & left_3, vertex & left_4,
                                          vertex & right_1, vertex & right_2,
                                          vertex & right_3, vertex & right_4);

        static void split_quadratic_bezier_at(unsigned int t,
                                              precision range_bits,
                                              const vertex *points,
                                              precision sub_pixel_bits,
                                              vertex & left_1, vertex & left_2, vertex & left_3,
                                              vertex & right_1, vertex & right_2, vertex & right_3);

        static void quadratic_to_cubic_bezier(const vertex *points,
                                              vertex &p0, vertex &p1,
                                              vertex &p2, vertex &p3);

        static unsigned int compute_cubic_bezier_flatness(const vertex *points, precision precision);
        static unsigned int compute_quadratic_bezier_flatness(const vertex *points, precision precision);
        static bool is_cubic_bezier_flat(const vertex *points, precision precision, unsigned int tolerance_distance_pixels);
        static bool is_quadratic_bezier_flat(const vertex *points, precision precision, unsigned int threshold);

    };

}

#include "../../../../src/curve_divider.cpp"