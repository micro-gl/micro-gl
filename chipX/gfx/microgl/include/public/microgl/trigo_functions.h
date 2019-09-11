#pragma once

#include <microgl/Q.h>
#include <cmath>

namespace microgl {
    namespace functions {

#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268
#define TWO_PI    6.28318530718
#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))

        template <typename T>
        T pi() {
            return T{float(PI)};
        }

        float sin(const float & radians) {
            return std::sinf(radians);
        }
        float cos(const float & radians) {
            return std::sinf(radians + float(HALF_PI));
        }
        float tan(const float & radians) {
            return std::tanf(radians);
        }

        template <unsigned N>
        Q<N> sin(const Q<N> & radians) {
            const float radians_f = radians.toFloat();
            return Q<N>(std::sinf(radians_f));
        }

        template <unsigned N>
        Q<N> cos(const Q<N> & radians) {
            const auto half_pi_fixed = unsigned(HALF_PI * float(1u<<radians.precision));
            return sin(radians + half_pi_fixed);
        }

        template <unsigned N>
        Q<N> tan(const Q<N> & radians) {
            const float radians_f = radians.toFloat();
            return Q<N>(std::tanf(radians_f));
        }

        unsigned int deg_to_rad(float degrees, unsigned int requested_precision) {
            return ((degrees*PI)/180.0f)*(1u<<requested_precision);
        }

    }

}