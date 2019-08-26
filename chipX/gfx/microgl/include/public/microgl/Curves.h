#pragma once

#include <microgl/vec2.h>
using namespace microgl;

namespace curves {

    class BezierCurve {
    public:
        using precision = unsigned char;
        enum class Type {
            Quadratic, Cubic
        };

        explicit BezierCurve(vec2_32i * $points, precision $precision, Type $type);

        void evaluate(unsigned int t,
                      precision range_bits,
                      vec2_32i & output);

        void evaluate(float t,
                      vec2_32i & output);

    private:

        void internal_evaluate_quadratic_bezier_at(unsigned int t,
                                                    precision range_bits,
                                                    vec2_32i & output);

        void internal_evaluate_cubic_bezier_at(unsigned int t,
                                                precision range_bits,
                                                vec2_32i & output);

        vec2_32i * _points;
        Type _type;
        unsigned char _precision;
    };

}