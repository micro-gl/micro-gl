#include <microgl/tesselation/curve_divider.h>

namespace tessellation {
    template <typename number>
    bool curve_divider<number>::is_cubic_bezier_flat(const vertex *points, curve_divider::precision precision,
                                                  unsigned int tolerance_distance_pixels) {
        return (compute_cubic_bezier_flatness(points, precision) <
                16*((tolerance_distance_pixels*tolerance_distance_pixels)<<(precision<<1)));
//        return (compute_cubic_bezier_flatness(points, precision) < threshold);
    }

    template <typename number>
    unsigned int
    curve_divider<number>::compute_quadratic_bezier_flatness(const vertex *points, curve_divider::precision precision) {
        vertex cubic[4];

        // convert quadratic to cubic
        quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);

        return compute_cubic_bezier_flatness(cubic, precision);
    }

    template <typename number>
    bool curve_divider<number>::is_quadratic_bezier_flat(const vertex *points, curve_divider::precision precision,
                                                      unsigned int threshold) {
        return (compute_quadratic_bezier_flatness(points, precision) < threshold);
    }

    template <typename number>
    unsigned int
    curve_divider<number>::compute_cubic_bezier_flatness(const vertex *points, curve_divider::precision precision) {

        const vertex &p1 = points[0];
        const vertex &cp1 = points[1];
        const vertex &cp2 = points[2];
        const vertex &p2 = points[3];

        int ux = 3 * cp1.x - 2 * p1.x - p2.x;
        int uy = 3 * cp1.y - 2 * p1.y - p2.y;
        int vx = 3 * cp2.x - 2 * p2.x - p1.x;
        int vy = 3 * cp2.y - 2 * p2.y - p1.y;

        ux *= ux;
        uy *= uy;
        vx *= vx;
        vy *= vy;

        if( ux < vx )  ux = vx;
        if( uy < vy )  uy = vy;

        return ((ux + uy));//>>(precision<<1));
    }

    template <typename number>
    void curve_divider<number>::quadratic_to_cubic_bezier(const vertex *points, vertex &p0, vertex &p1, vertex &p2,
                                                       vertex &p3) {

        p0 = points[0];
        p3 = points[2];

        // simple lerp 2/3 inside
        p1.x = points[0].x + ((points[1].x - points[0].x)*2)/3;
        p1.y = points[0].y + ((points[1].y - points[0].y)*2)/3;

        // simple lerp 2/3 inside
        p2.x = points[2].x + ((points[1].x - points[2].x)*2)/3;
        p2.y = points[2].y + ((points[1].y - points[2].y)*2)/3;
    }

    template <typename number>
    void curve_divider<number>::split_quadratic_bezier_at(const unsigned int t, const curve_divider::precision range_bits,
                                                       const vertex *points,
                                                       const curve_divider::precision sub_pixel_bits, vertex &left_1,
                                                       vertex &left_2, vertex &left_3, vertex &right_1,
                                                       vertex &right_2, vertex &right_3) {

        const vertex &p1 = points[0];
        const vertex &p2 = points[1];
        const vertex &p3 = points[2];

        vertex p12 = lerp_fixed(t, p1, p2);
        vertex p23 = lerp_fixed(t, p2, p3);

        vertex p123 = lerp_fixed(t, p12, p23);

        left_1 = p1;
        left_2 = p12;
        left_3 = p123;

        right_1 = p123;
        right_2 = p23;
        right_3 = p3;
    }

    template <typename number>
    void curve_divider<number>::split_cubic_bezier_at(const unsigned int t,
                                                   const curve_divider::precision range_bits,
                                                   const vertex *points,
                                                   const curve_divider::precision sub_pixel_bits, vertex &left_1,
                                                   vertex &left_2, vertex &left_3, vertex &left_4, vertex &right_1,
                                                   vertex &right_2, vertex &right_3, vertex &right_4) {

        const vertex &p1 = points[0];
        const vertex &p2 = points[1];
        const vertex &p3 = points[2];
        const vertex &p4 = points[3];

        vertex p12 = lerp_fixed(t, p1, p2);
        vertex p23 = lerp_fixed(t, p2, p3);
        vertex p34 = lerp_fixed(t, p3, p4);

        vertex p123 = lerp_fixed(t, p12, p23);
        vertex p234 = lerp_fixed(t, p23, p34);

        vertex p1234 = lerp_fixed(t, p123, p234);

        left_1 = p1;
        left_2 = p12;
        left_3 = p123;
        left_4 = p1234;

        right_1 = p1234;
        right_2 = p234;
        right_3 = p34;
        right_4 = p4;
    }

    template <typename number>
    void curve_divider<number>::evaluate_cubic_bezier_at(const unsigned int t,
                                                      const curve_divider::precision range_bits,
                                                      const vertex *points,
                                                      const curve_divider::precision sub_pixel_bits,
                                                      vertex &output) {

//        unsigned int resolution = range_bits;
//        unsigned int resolution_triple = resolution*3;
//        unsigned int N_SEG = (1 << resolution); // 64 resolution
//
//        // (n-t)^2 => n*n, t*t, n*t
//        // (n-t)^3 => n*n*n, t*t*n, n*n*t, t*t*t
//        //10
//        // todo: we can use a LUT if using more point batches
//        unsigned int comp = N_SEG - t;
//        unsigned int comp_times_comp = comp * comp;
//        unsigned int t_times_t = t * t;
//        unsigned int a = comp * comp_times_comp;
//        unsigned int b = 3 * (t * comp_times_comp);
//        unsigned int c = 3*t_times_t*comp;
//        unsigned int d = t*t_times_t;
//
//        output.x = ((long)a * points[0].x + (long)b * points[1].x + (long)c * points[2].x + (long)d * points[3].x)>>resolution_triple;
//        output.y = ((long)a * points[0].y + (long)b * points[1].y + (long)c * points[2].y + (long)d * points[3].y)>>resolution_triple;
    }

    template <typename number>
    void
    curve_divider<number>::evaluate_quadratic_bezier_directly_at(const number t,
                                                                const vertex *points,
                                                                vertex &output) {

        const number comp = number(1) - t;
        const number a = comp * comp;
        const number b = (t * comp) * number(2);
        const number c = t * t;

        output.x = (a * points[0].x + b * points[1].x + c * points[2].x);
        output.y = (a * points[0].y + b * points[1].y + c * points[2].y);

//        unsigned int resolution = range_bits;
//        unsigned int resolution_double = resolution<<1;
//        unsigned int N_SEG = (1 << resolution); // 64 resolution
//
//        unsigned int comp = N_SEG - t;
//        unsigned int a = comp * comp;
//        unsigned int b = (t * comp) << 1;
//        unsigned int c = t * t;
//        output.x = ((long)a * points[0].x + (long)b * points[1].x + (long)c * points[2].x) >> resolution_double;
//        output.y = ((long)a * points[0].y + (long)b * points[1].y + (long)c * points[2].y) >> resolution_double;
    }

    template <typename number>
    auto
    curve_divider<number>::lerp_fixed(
            const number t,
            const vertex &a, const vertex &b
            ) -> vertex {

        vertex r {};

        r.x = (a.x) + ((((b.x - a.x))*t));
        r.y = (a.y) + ((((b.y - a.y))*t));
//        r.y = (a.y<<bits) + (((b.y - a.y)*t)>>range_bits);

        return r;
    }

    template <typename number>
    void curve_divider<number>::compute(const vertex *points,
                                        curve_divider::precision precision,
                                         output &output,
                                         curve_divider::CurveDivisionAlgorithm algorithm,
                                         curve_divider::Type $type) {
        switch ($type) {
            case Type::Quadratic:
                sub_divide_quadratic_bezier(points, precision, output, algorithm);
                break;
            case Type::Cubic:
                sub_divide_cubic_bezier(points, precision, output, algorithm);
                break;
        }
    }

    template <typename number>
    void curve_divider<number>::sub_divide_cubic_bezier(const vertex *points,
                                                    curve_divider::precision precision,
                                                     output &output,
                                                     curve_divider::CurveDivisionAlgorithm algorithm) {

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

    template <typename number>
    void
    curve_divider<number>::sub_divide_quadratic_bezier(const vertex *points, curve_divider::precision precision,
                                                    output &output,
                                                    curve_divider::CurveDivisionAlgorithm algorithm) {

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
                uniform_sub_divide_quadratic_bezier(points, output);
                break;
            case CurveDivisionAlgorithm::Uniform_32:
                uniform_sub_divide_quadratic_bezier(points, output);
                break;
            case CurveDivisionAlgorithm::Uniform_64:
                uniform_sub_divide_quadratic_bezier(points, output);
                break;
        }

    }

    template <typename number>
    void curve_divider<number>::uniform_sub_divide_cubic_bezier(const vertex *points,
                                                             curve_divider::precision precision_point,
                                                             curve_divider::precision subdivision_bits,
                                                             output &output) {

        unsigned int segments = 1<<subdivision_bits;
        vertex current;

        for (unsigned int i=0; i <= segments; ++i) {
            evaluate_cubic_bezier_at(i, subdivision_bits, points, precision_point, current);
            output.push_back(current);
        }

    }

    template <typename number>
    void curve_divider<number>::uniform_sub_divide_quadratic_bezier(const vertex *points,
                                                                 output &output) {

        // this procedure was way more optimized for fixed point numbers
        // but decided, that to do a more generic thing. The cost is really small.
        // it's not like we are evaluating 100ks of point per second, so decided
        // to go generic.
        const index segments = 16;//1<<subdivision_bits;
        vertex current;
        const number step = number(1) / number(16);
        number accumulator = 0;

        for (index i=0; i <= segments; ++i) {
            evaluate_quadratic_bezier_directly_at(accumulator, points, current);
            accumulator += step;
            output.push_back(current);
        }

    }

    template <typename number>
    void curve_divider<number>::adaptive_sub_divide_cubic_bezier_internal(const vertex *points,
                                                                       curve_divider::precision precision,
                                                                       unsigned int tolerance_distance_pixels,
                                                                       output &output) {

        if(is_cubic_bezier_flat(points, precision, tolerance_distance_pixels)) {
            //            output.push_back(points[0]);
            output.push_back(points[3]);
        } else {
            vertex split_left[4];
            vertex split_right[4];

            split_cubic_bezier_at(1, 1, points, precision,
                                  split_left[0], split_left[1], split_left[2], split_left[3],
                                  split_right[0], split_right[1], split_right[2], split_right[3]
            );

            adaptive_sub_divide_cubic_bezier_internal(split_left, precision, tolerance_distance_pixels, output);
            adaptive_sub_divide_cubic_bezier_internal(split_right, precision, tolerance_distance_pixels, output);
        }

    }

    template <typename number>
    void curve_divider<number>::adaptive_sub_divide_cubic_bezier(const vertex *points,
                                                              curve_divider::precision precision,
                                                              unsigned int tolerance_distance_pixels,
                                                              output &output) {

        output.push_back(points[0]);

        adaptive_sub_divide_cubic_bezier_internal(points, precision, tolerance_distance_pixels, output);
    }

    template <typename number>
    void curve_divider<number>::adaptive_sub_divide_quadratic_bezier(const vertex *points,
                                                                  curve_divider::precision precision,
                                                                  unsigned int tolerance_distance_pixels,
                                                                  output &output) {

        vertex cubic[4];
        quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);
        adaptive_sub_divide_cubic_bezier(cubic, precision, tolerance_distance_pixels, output);
    }

}
