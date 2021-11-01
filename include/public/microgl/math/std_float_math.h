#pragma once

#include <microgl/math/base_math.h>
#include <cmath>

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
            return std::fmodf(numer, denom);
        }
        inline
        double mod(double numer, double denom) {
            return std::fmod(numer, denom);
        }

        inline float sqrt(const float val) {
            return std::sqrt(val);
        }
        inline double sqrt(const double val) {
            return std::sqrt(val);
        }

        inline
        float sin(const float radians) {
            return std::sin(radians);
        }
        inline
        double sin(const double radians) {
            return std::sin(radians);
        }

        inline
        float cos(const float radians) {
            return std::cos(radians);
        }
        inline
        double cos(const double radians) {
            return std::cos(radians);
        }

        inline
        float tan(const float radians) {
            return std::tan(radians);
        }
        inline
        double tan(const double radians) {
            return std::tan(radians);
        }
    }
}