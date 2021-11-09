/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "vec2.h"

namespace microtess {

    enum class CurveType {
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

    template<typename number, class container_type>
    class curve_divider {
    public:
        using vertex = microtess::vec2<number>;
        using output = container_type;
        using index = unsigned int;

        curve_divider()=delete;
        curve_divider(const curve_divider &)=delete;
        curve_divider(curve_divider &&)=delete;
        curve_divider & operator=(const curve_divider &)=delete;
        curve_divider & operator=(curve_divider &&)=delete;
        ~curve_divider()=delete;

        static void compute(const vertex *points,
                            output &output,
                            CurveDivisionAlgorithm algorithm,
                            CurveType $type) {
            switch ($type) {
                case CurveType::Quadratic:
                    sub_divide_quadratic_bezier(points, output, algorithm);
                    break;
                case CurveType::Cubic:
                    sub_divide_cubic_bezier(points, output, algorithm);
                    break;
            }
        }

    private:

        static void sub_divide_cubic_bezier(const vertex *points,
                                            output &output,
                                            CurveDivisionAlgorithm algorithm) {
            switch (algorithm) {
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small:
                    adaptive_sub_divide_cubic_bezier(points, 1, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium:
                    adaptive_sub_divide_cubic_bezier(points, 5, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Large:
                    adaptive_sub_divide_cubic_bezier(points, 10, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_16:
                    uniform_sub_divide_bezier_curve(points, 16, output, CurveType::Cubic);
                    break;
                case CurveDivisionAlgorithm::Uniform_32:
                    uniform_sub_divide_bezier_curve(points, 32, output, CurveType::Cubic);
                    break;
                case CurveDivisionAlgorithm::Uniform_64:
                    uniform_sub_divide_bezier_curve(points, 64, output, CurveType::Cubic);
                    break;
            }
        }

        static void sub_divide_quadratic_bezier(const vertex *points,
                                                output &output,
                                                CurveDivisionAlgorithm algorithm) {
            switch (algorithm) {
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small:
                    adaptive_sub_divide_quadratic_bezier(points, 1, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium:
                    adaptive_sub_divide_quadratic_bezier(points, 5, output);
                    break;
                case CurveDivisionAlgorithm::Adaptive_tolerance_distance_Large:
                    adaptive_sub_divide_quadratic_bezier(points, 10, output);
                    break;
                case CurveDivisionAlgorithm::Uniform_16:
                    uniform_sub_divide_bezier_curve(points, 16, output, CurveType::Quadratic);
                    break;
                case CurveDivisionAlgorithm::Uniform_32:
                    uniform_sub_divide_bezier_curve(points, 32, output, CurveType::Quadratic);
                    break;
                case CurveDivisionAlgorithm::Uniform_64:
                    uniform_sub_divide_bezier_curve(points, 64, output, CurveType::Quadratic);
                    break;
            }
        }


        static void uniform_sub_divide_bezier_curve(const vertex *points,
                                                    index segments,
                                                    output &output,
                                                    CurveType type) {
            // this procedure was way more optimized for fixed point math
            // but decided, that to do a more generic thing. The cost is really small.
            // it's not like we are evaluating 100ks of point per second, so decided
            // to go generic.
            vertex current;
            const number step = number(1) / number(int(segments));
            number accumulator = 0;
            bool is_quadratic = type == CurveType::Quadratic;

            for (index i = 0; i <= segments; ++i) {
                if (is_quadratic)
                    evaluate_quadratic_bezier_at(accumulator, points, current, true);
                else
                    evaluate_cubic_bezier_at(accumulator, points, current, true);

                accumulator += step;
                output.push_back(current);
            }
        }

        static void adaptive_sub_divide_cubic_bezier_internal(const vertex *points,
                                                              number tolerance_distance_pixels,
                                                              output &output) {
            if (is_cubic_bezier_flat(points, tolerance_distance_pixels)) {
                //            output.push_back(points[0]);
                output.push_back(points[3]);
            } else {
                vertex split_left[4];
                vertex split_right[4];

                split_cubic_bezier_at(number(1) / number(2), points,
                                      split_left[0], split_left[1], split_left[2], split_left[3],
                                      split_right[0], split_right[1], split_right[2], split_right[3]
                );

                adaptive_sub_divide_cubic_bezier_internal(split_left, tolerance_distance_pixels, output);
                adaptive_sub_divide_cubic_bezier_internal(split_right, tolerance_distance_pixels, output);
            }
        }

        static void adaptive_sub_divide_cubic_bezier(const vertex *points,
                                                     number tolerance_distance_pixels,
                                                     output &output) {
            output.push_back(points[0]);
            adaptive_sub_divide_cubic_bezier_internal(points, tolerance_distance_pixels, output);
        }

        static void adaptive_sub_divide_quadratic_bezier(const vertex *points,
                                                         number tolerance_distance_pixels,
                                                         output &output) {
            vertex cubic[4];
            quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);
            adaptive_sub_divide_cubic_bezier(cubic, tolerance_distance_pixels, output);
        }

        static vertex lerp(number t, const vertex &a, const vertex &b) {
            vertex r{};
            r.x = (a.x) + ((((b.x - a.x)) * t));
            r.y = (a.y) + ((((b.y - a.y)) * t));
            return r;
        }

        static void evaluate_quadratic_bezier_at(number t,
                                                 const vertex *points,
                                                 vertex &output,
                                                 bool use_De_Casteljau = true) {
            if (use_De_Casteljau) {
                const vertex &p1 = points[0];
                const vertex &p2 = points[1];
                const vertex &p3 = points[2];

                const vertex p12 = lerp(t, p1, p2);
                const vertex p23 = lerp(t, p2, p3);

                output = lerp(t, p12, p23);
            } else {
                const number comp = number(1) - t;
                const number a = comp * comp;
                const number b = (t * comp) * number(2);
                const number c = t * t;

                output.x = (a * points[0].x + b * points[1].x + c * points[2].x);
                output.y = (a * points[0].y + b * points[1].y + c * points[2].y);
            }
        }

        static void evaluate_cubic_bezier_at(number t,
                                             const vertex *points,
                                             vertex &output,
                                             bool use_De_Casteljau = true) {
            if (use_De_Casteljau) {
                const vertex &p1 = points[0];
                const vertex &p2 = points[1];
                const vertex &p3 = points[2];
                const vertex &p4 = points[3];

                const vertex p12 = lerp(t, p1, p2);
                const vertex p23 = lerp(t, p2, p3);
                const vertex p34 = lerp(t, p3, p4);

                const vertex p123 = lerp(t, p12, p23);
                const vertex p234 = lerp(t, p23, p34);

                output = lerp(t, p123, p234);
            } else {
                const number comp = number(1) - t;
                const number comp_times_comp = comp * comp;
                const number t_times_t = t * t;
                const number a = comp * comp_times_comp;
                const number b = number(3) * (t * comp_times_comp);
                const number c = number(3) * t_times_t * comp;
                const number d = t * t_times_t;

                output.x = (a * points[0].x + b * points[1].x + c * points[2].x + d * points[3].x);
                output.y = (a * points[0].y + b * points[1].y + c * points[2].y + d * points[3].y);
            }
        }

        static void split_cubic_bezier_at(number t,
                                          const vertex *points,
                                          vertex &left_1, vertex &left_2,
                                          vertex &left_3, vertex &left_4,
                                          vertex &right_1, vertex &right_2,
                                          vertex &right_3, vertex &right_4) {
            const vertex &p1 = points[0];
            const vertex &p2 = points[1];
            const vertex &p3 = points[2];
            const vertex &p4 = points[3];

            vertex p12 = lerp(t, p1, p2);
            vertex p23 = lerp(t, p2, p3);
            vertex p34 = lerp(t, p3, p4);

            vertex p123 = lerp(t, p12, p23);
            vertex p234 = lerp(t, p23, p34);

            vertex p1234 = lerp(t, p123, p234);

            left_1 = p1;
            left_2 = p12;
            left_3 = p123;
            left_4 = p1234;

            right_1 = p1234;
            right_2 = p234;
            right_3 = p34;
            right_4 = p4;
        }

        static void split_quadratic_bezier_at(number t,
                                              const vertex *points,
                                              vertex &left_1, vertex &left_2, vertex &left_3,
                                              vertex &right_1, vertex &right_2, vertex &right_3) {
            const vertex &p1 = points[0];
            const vertex &p2 = points[1];
            const vertex &p3 = points[2];

            vertex p12 = lerp(t, p1, p2);
            vertex p23 = lerp(t, p2, p3);

            vertex p123 = lerp(t, p12, p23);

            left_1 = p1;
            left_2 = p12;
            left_3 = p123;

            right_1 = p123;
            right_2 = p23;
            right_3 = p3;
        }

        static void quadratic_to_cubic_bezier(const vertex *points,
                                              vertex &p0, vertex &p1,
                                              vertex &p2, vertex &p3) {
            p0 = points[0];
            p3 = points[2];

            // simple lerp 2/3 inside
            p1.x = points[0].x + ((points[1].x - points[0].x) * 2) / 3;
            p1.y = points[0].y + ((points[1].y - points[0].y) * 2) / 3;

            // simple lerp 2/3 inside
            p2.x = points[2].x + ((points[1].x - points[2].x) * 2) / 3;
            p2.y = points[2].y + ((points[1].y - points[2].y) * 2) / 3;
        }

        static number compute_cubic_bezier_flatness(const vertex *points) {
            const vertex &p1 = points[0];
            const vertex &cp1 = points[1];
            const vertex &cp2 = points[2];
            const vertex &p2 = points[3];

            number ux = number(3) * cp1.x - number(2) * p1.x - p2.x;
            number uy = number(3) * cp1.y - number(2) * p1.y - p2.y;
            number vx = number(3) * cp2.x - number(2) * p2.x - p1.x;
            number vy = number(3) * cp2.y - number(2) * p2.y - p1.y;

            ux *= ux;
            uy *= uy;
            vx *= vx;
            vy *= vy;

            if (ux < vx) ux = vx;
            if (uy < vy) uy = vy;

            return ((ux + uy));
        }

        static number compute_quadratic_bezier_flatness(const vertex *points) {
            vertex cubic[4];
            // convert quadratic to cubic
            quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);
            return compute_cubic_bezier_flatness(cubic);
        }

        static bool is_cubic_bezier_flat(const vertex *points, number tolerance_distance_pixels) {
            return (compute_cubic_bezier_flatness(points) <
                    number(16) * (tolerance_distance_pixels * tolerance_distance_pixels));
        }

        static bool is_quadratic_bezier_flat(const vertex *points, unsigned int threshold) {
            return (compute_quadratic_bezier_flatness(points) < threshold);
        }

    };

}