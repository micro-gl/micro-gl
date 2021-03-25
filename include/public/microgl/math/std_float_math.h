#pragma once

#include <cmath>

namespace microgl {
    namespace math {
#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268

        inline int to_fixed(const float &val, unsigned char precision) {
            return int(val * float(int(1) << precision));
        }

        inline int to_fixed(const double &val, unsigned char precision) {
            return int(val * double(int(1) << precision));
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
            return std::sin(radians + float(HALF_PI));
        }
        inline
        double cos(const double radians) {
            return std::sin(radians + double(HALF_PI));
        }
        inline
        float tan(const float radians) {
            return std::tan(radians);
        }
        inline
        double tan(const double radians) {
            return std::tan(radians);
        }

        inline
        float deg_to_rad(float degrees) {
            return ((degrees*PI)/180.0f);
        }
        inline
        double deg_to_rad(double degrees) {
            return ((degrees*PI)/180.0f);
        }

        inline
        float mod(float numer, float denom) {
            return std::fmodf(numer, denom);
        }
        inline
        double mod(double numer, double denom) {
            return std::fmod(numer, denom);
        }

    }

}