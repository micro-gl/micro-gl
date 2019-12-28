#pragma once

#include <microgl/Q.h>
#include <cmath>

namespace microgl {

    class math {
    public:
#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268
#define TWO_PI    6.28318530718
#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))

        static int to_fixed(const float &val, unsigned char precision) {
            return int(val*float(1u<<precision));
        }

        template<unsigned N>
        static int to_fixed(const Q<N> & val, unsigned char precision) {
            return int(val.toFixed(precision));
        }

        static float sqrt(const float &val) {
            return sqrtf(val);
        }

        template<unsigned N>
        static int sqrt(const Q<N> & val) {
            return sqrt_64(val.value());
        }

        template <typename T>
        static T pi() {
            return T{float(PI)};
        }

        static
        float sin(const float & radians) {
            return std::sinf(radians);
        }
        static
        float cos(const float & radians) {
            return std::sinf(radians + float(HALF_PI));
        }
        static
        float tan(const float & radians) {
            return std::tanf(radians);
        }

        template <unsigned N>
        static
        Q<N> sin(const Q<N> & radians) {
            const float radians_f = radians.toFloat();
            return Q<N>(std::sinf(radians_f));
        }

        // todo: this is wrong
        template <unsigned N>
        static
        Q<N> cos(const Q<N> & radians) {
            const auto half_pi_fixed = unsigned(HALF_PI * float(1u<<radians.precision));
            return sin(radians + half_pi_fixed);
        }

        template <unsigned N>
        static
        Q<N> tan(const Q<N> & radians) {
            const float radians_f = radians.toFloat();
            return Q<N>(std::tanf(radians_f));
        }

        template <unsigned N>
        static
        Q<N> deg_to_rad(const Q<N> & degrees) {
            using q = Q<N>;
            return (degrees*math::pi<q>())/q(180);
        }

        static
        float deg_to_rad(float degrees) {
            return ((degrees*math::pi<float>())/180.0f);
        }

        static
        float mod(float numer, float denom) {
            return fmodf(numer, denom);
        }

        template <unsigned N>
        static
        Q<N> mod(const Q<N> & numer, const Q<N> & denom) {
            return numer%denom;
        }

        static
        float abs_(const float &val) {
            return fabs(val);
        }

        template <unsigned N>
        static
        Q<N> abs_(const Q<N> & val) {
            return Q<N>(val.value()<0 ? -val.value() : val.value(), N);
        }

    private:
        static
        uint32_t sqrt_64(uint64_t a_nInput) {
            uint64_t op  = a_nInput;
            uint64_t res = 0;
            uint64_t one = uint64_t(1u) << 62; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

            // "one" starts at the highest power of four <= than the argument.
            while (one > op)
            {
                one >>= 2;
            }

            while (one != 0)
            {
                if (op >= res + one)
                {
                    op = op - (res + one);
                    res = res + ( one<<1);
                }
                res >>= 1;
                one >>= 2;
            }
            return res;
        }
    };



}