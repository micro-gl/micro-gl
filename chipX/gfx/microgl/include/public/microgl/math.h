#pragma once

#include <microgl/Q.h>
#include <cmath>

namespace microgl {

    class math {
    private:
        using cfr = const float &;
        template <unsigned N>
        using cqr = const Q<N> &;
        using l64= long long;

    public:
#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268
#define TWO_PI    6.28318530718
#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))

        static l64 to_fixed(const float &val, unsigned char precision) {
            return l64(val*float(1u<<precision));
        }

        static l64 to_fixed(const double &val, unsigned char precision) {
            return l64(val*double(1u<<precision));
        }

        static l64 to_fixed(const unsigned &val, unsigned char precision) {
            return (val<<precision);
        }

        static l64 to_fixed(const int &val, unsigned char precision) {
            return (val<<precision);
        }

        template<unsigned N>
        static l64 to_fixed(const Q<N> & val, unsigned char precision) {
            return l64(val.toFixed(precision));
        }

        static float sqrt(const float &val) {
            return sqrtf(val);
        }

        static l64 sqrt(const l64 &val) {
            return sqrt_64(val);
        }

        template<unsigned N>
        static Q<N> sqrt(const Q<N> & val) {
            return Q<N>((sqrt_64(val.value())), N>>1);
        }

        static float distance(cfr p0_x, cfr p0_y, cfr p1_x, cfr p1_y) {
            auto dx= p0_x-p1_x;
            auto dy= p0_y-p1_y;
            return length(dx, dy);
        }

        static float length(cfr p_x, cfr p_y) {
            return sqrtf(p_x*p_x + p_y*p_y);
        }

        template<unsigned N>
        static Q<N> distance(cqr<N> p0_x, cqr<N> p0_y, cqr<N> p1_x, cqr<N> p1_y) {
            auto dx= p0_x-p1_x;
            auto dy= p0_y-p1_y;
            // notice, that for fixed points we don't use the regular multiplication.
            // why? suppose x is p bits-> x^2 is 2p bits, then squaring it would give p bits again
            // which is what we want. But, if we use regular fixed point multiplication, then
            // x*x would yield a p bit number (because we do (x*x)>>p) and then squaring would yield
            // a p/2 bits result which is not what we wanted
//            return sqrt_64(dx.value()*dx.value() + dy.value()*dy.value());
            return length(dx, dy);
        }

        template<unsigned N>
        static Q<N> length(cqr<N> p_x, cqr<N> p_y) {
            return Q<N>(sqrt_64(int64_t (p_x.value())*p_x.value() + int64_t(p_y.value())*p_y.value()), N);
        }

        static int distance(const int & p0_x, const int & p0_y, const int & p1_x, const int & p1_y) {
            auto dx= p0_x-p1_x;
            auto dy= p0_y-p1_y;
            return length(dx, dy);
        }
        static int length(const int &p_x, const int & p_y) {
            return sqrt_64(l64(p_x)*p_x + l64(p_y)*p_y);
        }

        static int distance(const l64 & p0_x, const l64 & p0_y, const l64 & p1_x, const l64 & p1_y) {
            auto dx= p0_x-p1_x;
            auto dy= p0_y-p1_y;
            return length(dx, dy);
        }
        static int length(const l64 &p_x, const l64 & p_y) {
            return sqrt_64(l64(p_x)*p_x + l64(p_y)*p_y);
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

        template <unsigned N>
        static
        Q<N> cos(const Q<N> & radians) {
            const auto half_pi = Q<N>(HALF_PI);
            return sin(radians + half_pi);
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