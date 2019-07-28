//
// Created by Tomer Shalev on 2019-07-27.
//

#include <Curves.h>

namespace curves {

    /*
    function splitCurveAtT(p,t, debug) {
        let p1 = p[0]
        let p2 = p[1]
        let p3 = p[2]
        let p4 = p[3]
        let p12 = midpoint(p1,p2,t)
        let p23 = midpoint(p2,p3,t)
        let p34 = midpoint(p4,p3,t)
        let p123 = midpoint(p12,p23,t)
        let p234 = midpoint(p23, p34,t)
        let p1234 = { x: (p234.x-p123.x)*t+p123.x,
                y: (p234.y-p123.y)*t+p123.y}
        return [[p1, p12, p123, p1234],[p1234,p234,p34,p4]]
    }
    function midpoint(p1,p2,t) {
        return { x: (p2.x-p1.x)*t+p1.x, y: (p2.y-p1.y)*t+p1.y}
    }

    */

    vec2_32i lerp_fixed(const vec2_32i &a, const vec2_32i &b, int t, uint8_t range_bits) {
        vec2_32i r {};

        r.x = a.x + (((b.x - a.x)*t)>>range_bits);
        r.y = a.y + (((b.y - a.y)*t)>>range_bits);

        return r;
    }

    void split_cubic_bezier_at(const unsigned int t, const uint8_t range_bits,
                                      const vec2_32i *points, const uint8_t sub_pixel_bits,
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
                                          const vec2_32i *points, const uint8_t sub_pixel_bits,
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


}