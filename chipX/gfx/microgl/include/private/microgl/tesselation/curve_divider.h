#pragma once

#include <microgl/vec2.h>
#include <microgl/dynamic_array.h>

using namespace microgl;

namespace tessellation {

    template <typename number>
    class curve_divider {
    public:
        using vertex = vec2<number>;
        using output = dynamic_array<vertex>;
        using index = unsigned int;
//        static number HALF = number(1)/number(2);

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
                            output &output,
                            CurveDivisionAlgorithm algorithm,
                            Type $type);

    private:

        static void sub_divide_cubic_bezier(const vertex *points,
                                            output &output,
                                            curve_divider::CurveDivisionAlgorithm algorithm);

        static void sub_divide_quadratic_bezier(const vertex *points,
                                                output &output,
                                                CurveDivisionAlgorithm algorithm);


        static void uniform_sub_divide_bezier_curve(const vertex *points,
                                                    index segments,
                                                    output &output,
                                                    Type type);

        static void adaptive_sub_divide_cubic_bezier_internal(const vertex *points,
                                                              number tolerance_distance_pixels,
                                                              output &output
                                                       );

        static void adaptive_sub_divide_cubic_bezier(const vertex *points,
                                                     number tolerance_distance_pixels,
                                                     output &output
                                              );

        static void adaptive_sub_divide_quadratic_bezier(const vertex *points,
                                                         number tolerance_distance_pixels,
                                                         output &output
                                                  );

        static vertex lerp(number t, const vertex &a, const vertex &b);

        static void evaluate_quadratic_bezier_at(number t,
                                                 const vertex *points,
                                                 vertex & output,
                                                 bool use_De_Casteljau=true);

        static void evaluate_cubic_bezier_at(number t,
                                             const vertex *points,
                                             vertex & output,
                                             bool use_De_Casteljau=true);

        static void split_cubic_bezier_at(number t,
                                          const vertex *points,
                                          vertex & left_1, vertex & left_2,
                                          vertex & left_3, vertex & left_4,
                                          vertex & right_1, vertex & right_2,
                                          vertex & right_3, vertex & right_4);

        static void split_quadratic_bezier_at(number t,
                                              const vertex *points,
                                              vertex & left_1, vertex & left_2, vertex & left_3,
                                              vertex & right_1, vertex & right_2, vertex & right_3);

        static void quadratic_to_cubic_bezier(const vertex *points,
                                              vertex &p0, vertex &p1,
                                              vertex &p2, vertex &p3);

        static number compute_cubic_bezier_flatness(const vertex *points);
        static number compute_quadratic_bezier_flatness(const vertex *points);
        static bool is_cubic_bezier_flat(const vertex *points, number tolerance_distance_pixels);
        static bool is_quadratic_bezier_flat(const vertex *points, unsigned int threshold);

    };

}

#include "curve_divider.cpp"