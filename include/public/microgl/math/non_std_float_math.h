#pragma once

#include <microgl/math/base_math.h>

namespace microgl {
    namespace math {

        inline int to_fixed(const float val, unsigned char precision) {
            return int(val * float(int(1) << precision));
        }
        inline int to_fixed(const double val, unsigned char precision) {
            return int(val * double(int(1) << precision));
        }

        inline
        float mod(float numer, float denom) {
            return microgl::math::mod_cpu<float>(numer, denom);
        }
        inline
        double mod(double numer, double denom) {
            return microgl::math::mod_cpu<double>(numer, denom);
        }

        inline float sqrt(const float val) {
            return microgl::math::sqrt_cpu<float>(val, 0.0001f);
        }
        inline double sqrt(const double val) {
            return microgl::math::sqrt_cpu<double>(val, 0.0000001f);
        }

        inline
        float sin(const float radians) {
            return microgl::math::sin_bhaskara_cpu<float>(radians);
        }
        inline
        double sin(const double radians) {
            return microgl::math::sin_bhaskara_cpu<double>(radians);
        }

        inline
        float cos(const float radians) {
            return microgl::math::cos_bhaskara_cpu<float>(radians);
        }
        inline
        double cos(const double radians) {
            return microgl::math::cos_bhaskara_cpu<double>(radians);
        }

        inline
        float tan(const float radians) {
            return microgl::math::tan_bhaskara_cpu<float>(radians);
        }
        inline
        double tan(const double radians) {
            return microgl::math::tan_bhaskara_cpu<double>(radians);
        }

    }

}