#pragma once

#include <microgl/tesselation/BezierCurveDivider.h>

namespace tessellation {
    bool BezierCurveDivider::is_cubic_bezier_flat(const vec2_32i *points, BezierCurveDivider::precision precision,
                                                  unsigned int tolerance_distance_pixels) {
        return (compute_cubic_bezier_flatness(points, precision) <
                16*((tolerance_distance_pixels*tolerance_distance_pixels)<<(precision<<1)));
//        return (compute_cubic_bezier_flatness(points, precision) < threshold);
    }

    unsigned int
    BezierCurveDivider::compute_quadratic_bezier_flatness(const vec2_32i *points, BezierCurveDivider::precision precision) {
        vec2_32i cubic[4];

        // convert quadratic to cubic
        quadratic_to_cubic_bezier(points, cubic[0], cubic[1], cubic[2], cubic[3]);

        return compute_cubic_bezier_flatness(cubic, precision);
    }

    bool BezierCurveDivider::is_quadratic_bezier_flat(const vec2_32i *points, BezierCurveDivider::precision precision,
                                                      unsigned int threshold) {
        return (compute_quadratic_bezier_flatness(points, precision) < threshold);
    }

    unsigned int
    BezierCurveDivider::compute_cubic_bezier_flatness(const vec2_32i *points, BezierCurveDivider::precision precision) {

        const vec2_32i &p1 = points[0];
        const vec2_32i &cp1 = points[1];
        const vec2_32i &cp2 = points[2];
        const vec2_32i &p2 = points[3];

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

    void BezierCurveDivider::quadratic_to_cubic_bezier(const vec2_32i *points, vec2_32i &p0, vec2_32i &p1, vec2_32i &p2,
                                                       vec2_32i &p3) {

        p0 = points[0];
        p3 = points[2];

        // simple lerp 2/3 inside
        p1.x = points[0].x + ((points[1].x - points[0].x)*2)/3;
        p1.y = points[0].y + ((points[1].y - points[0].y)*2)/3;

        // simple lerp 2/3 inside
        p2.x = points[2].x + ((points[1].x - points[2].x)*2)/3;
        p2.y = points[2].y + ((points[1].y - points[2].y)*2)/3;
    }

    void BezierCurveDivider::split_quadratic_bezier_at(const unsigned int t, const BezierCurveDivider::precision range_bits,
                                                       const vec2_32i *points,
                                                       const BezierCurveDivider::precision sub_pixel_bits, vec2_32i &left_1,
                                                       vec2_32i &left_2, vec2_32i &left_3, vec2_32i &right_1,
                                                       vec2_32i &right_2, vec2_32i &right_3) {

        const vec2_32i &p1 = points[0];
        const vec2_32i &p2 = points[1];
        const vec2_32i &p3 = points[2];

        vec2_32i p12 = lerp_fixed(t, range_bits, p1, p2, sub_pixel_bits);
        vec2_32i p23 = lerp_fixed(t, range_bits, p2, p3, sub_pixel_bits);

        vec2_32i p123 = lerp_fixed(t, range_bits, p12, p23, sub_pixel_bits);

        left_1 = p1;
        left_2 = p12;
        left_3 = p123;

        right_1 = p123;
        right_2 = p23;
        right_3 = p3;
    }

    void BezierCurveDivider::split_cubic_bezier_at(const unsigned int t, const BezierCurveDivider::precision range_bits,
                                                   const vec2_32i *points,
                                                   const BezierCurveDivider::precision sub_pixel_bits, vec2_32i &left_1,
                                                   vec2_32i &left_2, vec2_32i &left_3, vec2_32i &left_4, vec2_32i &right_1,
                                                   vec2_32i &right_2, vec2_32i &right_3, vec2_32i &right_4) {

        const vec2_32i &p1 = points[0];
        const vec2_32i &p2 = points[1];
        const vec2_32i &p3 = points[2];
        const vec2_32i &p4 = points[3];

        vec2_32i p12 = lerp_fixed(t, range_bits, p1, p2, sub_pixel_bits);
        vec2_32i p23 = lerp_fixed(t, range_bits, p2, p3, sub_pixel_bits);
        vec2_32i p34 = lerp_fixed(t, range_bits, p3, p4, sub_pixel_bits);

        vec2_32i p123 = lerp_fixed(t, range_bits, p12, p23, sub_pixel_bits);
        vec2_32i p234 = lerp_fixed(t, range_bits, p23, p34, sub_pixel_bits);

        vec2_32i p1234 = lerp_fixed(t, range_bits, p123, p234, sub_pixel_bits);

        left_1 = p1;
        left_2 = p12;
        left_3 = p123;
        left_4 = p1234;

        right_1 = p1234;
        right_2 = p234;
        right_3 = p34;
        right_4 = p4;
    }

    void BezierCurveDivider::evaluate_cubic_bezier_at(const unsigned int t, const BezierCurveDivider::precision range_bits,
                                                      const vec2_32i *points,
                                                      const BezierCurveDivider::precision sub_pixel_bits, vec2_32i &output) {

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

    void
    BezierCurveDivider::evaluate_quadratic_bezier_at(const unsigned int t, const BezierCurveDivider::precision range_bits,
                                                     const vec2_32i *points,
                                                     const BezierCurveDivider::precision sub_pixel_bits, vec2_32i &output) {

        unsigned int resolution = range_bits;
        unsigned int resolution_double = resolution<<1;
        unsigned int N_SEG = (1 << resolution); // 64 resolution

        unsigned int comp = N_SEG - t;
        unsigned int a = comp * comp;
        unsigned int b = (t * comp) << 1;
        unsigned int c = t * t;
        output.x = ((long)a * points[0].x + (long)b * points[1].x + (long)c * points[2].x) >> resolution_double;
        output.y = ((long)a * points[0].y + (long)b * points[1].y + (long)c * points[2].y) >> resolution_double;
    }

    vec2_32i
    BezierCurveDivider::lerp_fixed(int t, BezierCurveDivider::precision range_bits, const vec2_32i &a, const vec2_32i &b,
                                   BezierCurveDivider::precision point_precision) {
        vec2_32i r {};

        uint8_t bits = point_precision - point_precision;

        r.x = (a.x<<bits) + ((((b.x - a.x)<<bits)*t)>>range_bits);
        r.y = (a.y<<bits) + ((((b.y - a.y)<<bits)*t)>>range_bits);
//        r.y = (a.y<<bits) + (((b.y - a.y)*t)>>range_bits);

        r.x >>= bits;
        r.y >>= bits;

        return r;
    }

}
