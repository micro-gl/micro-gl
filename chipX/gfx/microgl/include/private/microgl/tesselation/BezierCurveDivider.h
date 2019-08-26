#pragma once

#include <microgl/vec2.h>
#include <microgl/StaticArray.h>
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

        template<unsigned N>
        static void compute(const vec2_32i *points,
                            precision precision,
                            StaticArray<vec2_32i, N> &output,
                            CurveDivisionAlgorithm algorithm,
                            Type $type) {
            switch ($type) {
                case Type::Quadratic:
                    sub_divide_quadratic_bezier(points, precision, output, algorithm);
                    break;
                case Type::Cubic:
                    sub_divide_cubic_bezier(points, precision, output, algorithm);
                    break;
            }
        }

    private:
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

        template<unsigned int N>
        static void sub_divide_cubic_bezier(const vec2_32i *points, BezierCurveDivider::precision precision,
                                                         StaticArray<vec2_32i, N> &output,
                                                         BezierCurveDivider::CurveDivisionAlgorithm algorithm) {

            switch(algorithm) {
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small:
                    adaptive_sub_divide_cubic_bezier(points, precision, 1, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium:
                    adaptive_sub_divide_cubic_bezier(points, precision, 5, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Large:
                    adaptive_sub_divide_cubic_bezier(points, precision, 10, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_16:
                    uniform_sub_divide_cubic_bezier(points, precision, 4, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_32:
                    uniform_sub_divide_cubic_bezier(points, precision, 5, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_64:
                    uniform_sub_divide_cubic_bezier(points, precision, 6, output);
                    break;
            }

        }

        template<unsigned N>
        static void sub_divide_quadratic_bezier(const vec2_32i *points,
                                                precision precision,
                                                StaticArray<vec2_32i, N> &output,
                                                CurveDivisionAlgorithm algorithm) {

            switch(algorithm) {
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small:
                    adaptive_sub_divide_quadratic_bezier(points, precision, 1, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium:
                    adaptive_sub_divide_quadratic_bezier(points, precision, 5, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Large:
                    adaptive_sub_divide_quadratic_bezier(points, precision, 10, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_16:
                    uniform_sub_divide_quadratic_bezier(points, precision, 4, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_32:
                    uniform_sub_divide_quadratic_bezier(points, precision, 5, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_64:
                    uniform_sub_divide_quadratic_bezier(points, precision, 6, output);
                    break;
            }

        }


        template<unsigned N>
        static void uniform_sub_divide_cubic_bezier(const vec2_32i *points,
                                                    precision precision_point,
                                                    precision subdivision_bits,
                                                    StaticArray<vec2_32i, N> &output
                                             ) {

            unsigned int segments = 1<<subdivision_bits;
            vec2_32i current;

            for (unsigned int i=0; i <= segments; ++i) {
                evaluate_cubic_bezier_at(i, subdivision_bits, points, precision_point, current);
                output.push_back(current);
            }

        }

        template<unsigned N>
        static void uniform_sub_divide_quadratic_bezier(const vec2_32i *points,
                                                        precision precision_point,
                                                        precision subdivision_bits,
                                                        StaticArray<vec2_32i, N> &output
                                                 ) {

            unsigned int segments = 1<<subdivision_bits;
            vec2_32i current;

            for (unsigned int i=0; i <= segments; ++i) {
                evaluate_quadratic_bezier_at(i, subdivision_bits, points, precision_point, current);
                output.push_back(current);
            }

        }

        template<unsigned N>
        static void adaptive_sub_divide_cubic_bezier_internal(const vec2_32i *points,
                                                              precision precision,
                                                              unsigned int tolerance_distance_pixels,
                                                              StaticArray<vec2_32i, N> &output
                                                       ) {

            if(is_cubic_bezier_flat(points, precision, tolerance_distance_pixels)) {
    //            output.push_back(points[0]);
                output.push_back(points[3]);
            } else {
                vec2_32i split_left[4];
                vec2_32i split_right[4];

                split_cubic_bezier_at(1, 1, points, precision,
                                      split_left[0], split_left[1], split_left[2], split_left[3],
                                      split_right[0], split_right[1], split_right[2], split_right[3]
                );

                adaptive_sub_divide_cubic_bezier_internal(split_left, precision, tolerance_distance_pixels, output);
                adaptive_sub_divide_cubic_bezier_internal(split_right, precision, tolerance_distance_pixels, output);
            }

        }

        template<unsigned N>
        static void adaptive_sub_divide_cubic_bezier(const vec2_32i *points,
                                                     precision precision,
                                                     unsigned int tolerance_distance_pixels,
                                                     StaticArray<vec2_32i, N> &output
                                              ) {

            output.push_back(points[0]);

            adaptive_sub_divide_cubic_bezier_internal(points, precision, tolerance_distance_pixels, output);
        }

        template<unsigned N>
        static void adaptive_sub_divide_quadratic_bezier(const vec2_32i *points,
                                                         precision precision,
                                                         unsigned int tolerance_distance_pixels,
                                                         StaticArray<vec2_32i, N> &output
                                                  ) {

            vec2_32i cubic[4];
            quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);
            adaptive_sub_divide_cubic_bezier(cubic, precision, tolerance_distance_pixels, output);
        }

    };

}
