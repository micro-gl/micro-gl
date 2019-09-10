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

//        template <typename T>
//        T compute_sin(const T & radians) {
//
//        }

        void one(int &val) {
            val = 1;
        }
        void one(float &val) {
            val = 1.0f;
        }
        template <unsigned N>
        void one(Q<N> & val) {
            val = Q<N>{1<<N};
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
            const auto half_pi_fixed = unsigned(HALF_PI * float(1u<<radians.precision()));
            return sin(radians + half_pi_fixed);
        }

//        template <unsigned N>
//        Q<10> sin(const Q<N> & radians) {
//            auto precision_out = float(1u<<10);
//            const float radians_f = radians.toFloat();
//
//            return int(std::sinf(radians_f)*precision_out);
//        }
//
//        template <unsigned N>
//        Q<10> cos(const Q<N> & radians) {
//            const auto half_pi_fixed = unsigned(HALF_PI * float(1u<<radians.precision()));
//            return sin(radians + half_pi_fixed);
//        }
//
        uint deg_to_rad(float degrees, uint requested_precision) {
            return ((degrees*PI)/180.0f)*(1u<<requested_precision);
        }

//        int compute_sin_fixed(const int radians,
//                              precision_t input_precision,
//                              precision_t output_precision) {
//            auto precision_m = float(1u<<input_precision);
//            auto precision_out = float(1u<<output_precision);
//            float radians_f = float(radians) / precision_m;
//
//            return int(std::sin(radians_f)*precision_out);
//        }

//        int compute_cos_fixed(const int radians,
//                              const precision_t input_precision,
//                              const precision_t output_precision) {
//            const auto half_pi_fixed = unsigned(HALF_PI * float(1u<<input_precision));
//            return compute_sin_fixed(radians + half_pi_fixed, input_precision, output_precision);
//        }


    }
}