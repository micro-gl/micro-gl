//
// Created by Tomer Shalev on 2019-07-27.
//

#include <Curves.h>

namespace curves {

    vec2_32i lerp_fixed(const vec2_32i &a, const vec2_32i &b, int t, uint8_t range_bits) {
        vec2_32i r {};

        r.x = a.x + (((b.x - a.x)*t)>>range_bits);
        r.y = a.y + (((b.y - a.y)*t)>>range_bits);

        return r;
    }

    void split_cubic_bezier_at(const unsigned int t, const uint8_t range_bits,
                               const vec2_32i *points,
                               const uint8_t sub_pixel_bits,
                               vec2_32i & left_1, vec2_32i & left_2,
                               vec2_32i & left_3, vec2_32i & left_4,
                               vec2_32i & right_1, vec2_32i & right_2,
                               vec2_32i & right_3, vec2_32i & right_4) {

        const vec2_32i &p1 = points[0];
        const vec2_32i &p2 = points[1];
        const vec2_32i &p3 = points[2];
        const vec2_32i &p4 = points[3];

        vec2_32i p12 = lerp_fixed(p1, p2, t, range_bits);
        vec2_32i p23 = lerp_fixed(p2, p3, t, range_bits);
        vec2_32i p34 = lerp_fixed(p3, p4, t, range_bits);

        vec2_32i p123 = lerp_fixed(p12, p23, t, range_bits);
        vec2_32i p234 = lerp_fixed(p23, p34, t, range_bits);

        vec2_32i p1234 = lerp_fixed(p123, p234, t, range_bits);

        left_1 = p1;
        left_2 = p12;
        left_3 = p123;
        left_4 = p1234;

        right_1 = p1234;
        right_2 = p234;
        right_3 = p34;
        right_4 = p4;
    }

    void split_quadratic_bezier_at(const unsigned int t, const uint8_t range_bits,
                                   const vec2_32i *points,
                                   const uint8_t sub_pixel_bits,
                                   vec2_32i & left_1, vec2_32i & left_2, vec2_32i & left_3,
                                   vec2_32i & right_1, vec2_32i & right_2, vec2_32i & right_3) {

        const vec2_32i &p1 = points[0];
        const vec2_32i &p2 = points[1];
        const vec2_32i &p3 = points[2];

        vec2_32i p12 = lerp_fixed(p1, p2, t, range_bits);
        vec2_32i p23 = lerp_fixed(p2, p3, t, range_bits);

        vec2_32i p123 = lerp_fixed(p12, p23, t, range_bits);

        left_1 = p1;
        left_2 = p12;
        left_3 = p123;

        right_1 = p123;
        right_2 = p23;
        right_3 = p3;
    }

    void evaluate_quadratic_bezier_at(const unsigned int t, const uint8_t range_bits,
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

    void evaluate_cubic_bezier_at(const unsigned int t, const uint8_t range_bits,
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


//    function flatness(curve) {
//        const p1 = curve[0]
//        const cp1 = curve[1]
//        const cp2 = curve[2]
//        const p2 = curve[3]
//        let ux = Math.pow( 3 * cp1.x - 2 * p1.x - p2.x, 2 );
//        let uy = Math.pow( 3 * cp1.y - 2 * p1.y - p2.y, 2 );
//        let vx = Math.pow( 3 * cp2.x - 2 * p2.x - p1.x, 2 );
//        let vy = Math.pow( 3 * cp2.y - 2 * p2.y - p1.y, 2 );
//        if( ux < vx )  ux = vx;
//        if( uy < vy )  uy = vy;
//        return ux + uy;
//    }


    void quadratic_to_cubic_bezier(const vec2_32i *points,
                                   vec2_32i &p0, vec2_32i &p1,
                                   vec2_32i &p2, vec2_32i &p3) {

        p0 = points[0];
        p3 = points[2];

        // simple lerp 2/3 inside
        p1.x = points[0].x + ((points[1].x - points[0].x)*2)/3;
        p1.y = points[0].y + ((points[1].y - points[0].y)*2)/3;

        // simple lerp 2/3 inside
        p2.x = points[2].x + ((points[1].x - points[2].x)*2)/3;
        p2.y = points[2].y + ((points[1].y - points[2].y)*2)/3;
    }

    unsigned int compute_cubic_bezier_flatness(const vec2_32i *points, uint8_t precision) {

        const vec2_32i &p1 = points[0];
        const vec2_32i &cp1 = points[1];
        const vec2_32i &cp2 = points[2];
        const vec2_32i &p2 = points[2];

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

        return ((ux + uy)>>(precision<<1));
    }

    unsigned int compute_quadratic_bezier_flatness(const vec2_32i *points, uint8_t precision) {
        vec2_32i cubic[4];

        // convert quadratic to cubic
        quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);

        return compute_cubic_bezier_flatness(cubic);
    }

    bool is_cubic_bezier_flat(const vec2_32i *points, uint8_t precision, unsigned int threshold) {
        return (compute_cubic_bezier_flatness(points, precision) < threshold);
    }

    bool is_quadratic_bezier_flat(const vec2_32i *points, uint8_t precision, unsigned int threshold) {
        return (compute_quadratic_bezier_flatness(points, precision) < threshold);
    }

    void uniform_sub_divide_cubic_bezier(const vec2_32i *points,
                                         uint8_t precision,
                                         uint8_t subdivision_bits,
                                         std::vector<vec2_32i> &output) {

        unsigned int segments = 1<<subdivision_bits;
        vec2_32i current;

        for (unsigned int i=0; i <= segments; ++i) {
            curves::evaluate_cubic_bezier_at(i, subdivision_bits, points, precision, current);
            output.push_back(current);
        }

    }

    void uniform_sub_divide_quadratic_bezier(const vec2_32i *points,
                                         uint8_t precision,
                                         uint8_t subdivision_bits,
                                         std::vector<vec2_32i> &output) {

        unsigned int segments = 1<<subdivision_bits;
        vec2_32i current;

        for (unsigned int i=0; i <= segments; ++i) {
            curves::evaluate_quadratic_bezier_at(i, subdivision_bits, points, precision, current);
            output.push_back(current);
        }

    }

    void adaptive_sub_divide_cubic_bezier(const vec2_32i *points,
                                      uint8_t precision,
                                      unsigned int threshold,
                                      std::vector<vec2_32i> &output) {

        if(is_cubic_bezier_flat(points, precision, threshold)) {
            output.push_back(points[0]);
            output.push_back(points[3]);
        } else {
            vec2_32i split_left[4];
            vec2_32i split_right[4];

            split_cubic_bezier_at(1, 1, points, precision,
                                  split_left[0], split_left[1], split_left[2], split_left[3],
                                  split_right[0], split_right[1], split_right[2], split_right[3]
            );

            adaptive_sub_divide_cubic_bezier(split_left, precision, threshold, output);
            adaptive_sub_divide_cubic_bezier(split_right, precision, threshold, output);
        }

    }

    void adaptive_sub_divide_quadratic_bezier(const vec2_32i *points,
                                              uint8_t precision,
                                              unsigned int threshold,
                                              std::vector<vec2_32i> &output) {

        vec2_32i cubic[4];
        quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);
        adaptive_sub_divide_cubic_bezier(cubic, precision, threshold, output);
    }

}