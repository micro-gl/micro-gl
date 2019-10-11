#include <microgl/Curves.h>

namespace curves {

    BezierCurve::BezierCurve(vec2_32i *$points, precision $precision, BezierCurve::Type $type) :
            _points{$points}, _type{$type}, _precision{$precision} {}

    void BezierCurve::evaluate(float t, vec2_32i &output) {
        precision p = 4;
        unsigned int t_ = int(t*(1<<p));

        evaluate(t_, p, output);
    }

    void BezierCurve::evaluate(const unsigned int t,
                               const precision range_bits,
                               vec2_32i &output) {
        if(_type==Type::Cubic)
            internal_evaluate_cubic_bezier_at(t, range_bits, output);
        else if(_type==Type::Quadratic)
            internal_evaluate_quadratic_bezier_at(t, range_bits, output);
    }

    void BezierCurve::internal_evaluate_quadratic_bezier_at(const unsigned int t,
                                                            const precision range_bits,
                                                            vec2_32i &output) {
        unsigned int resolution = range_bits;
        unsigned int resolution_double = resolution<<1;
        unsigned int N_SEG = (1 << resolution); // 64 resolution

        unsigned int comp = N_SEG - t;
        unsigned int a = comp * comp;
        unsigned int b = (t * comp) << 1;
        unsigned int c = t * t;

        output.x = ((long)a *_points[0].x + (long)b * _points[1].x + (long)c * _points[2].x) >> resolution_double;
        output.y = ((long)a *_points[0].y + (long)b * _points[1].y + (long)c * _points[2].y) >> resolution_double;
    }

    void
    BezierCurve::internal_evaluate_cubic_bezier_at(const unsigned int t, const precision range_bits, vec2_32i &output) {
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

        output.x = ((long)a * _points[0].x + (long)b * _points[1].x + (long)c * _points[2].x + (long)d * _points[3].x)>>resolution_triple;
        output.y = ((long)a * _points[0].y + (long)b * _points[1].y + (long)c * _points[2].y + (long)d * _points[3].y)>>resolution_triple;
    }

}