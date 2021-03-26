#pragma once

#include <microgl/Q.h>
#include <cmath>

namespace microgl {
    namespace math {

#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268

        template<unsigned N>
        inline int to_fixed(const Q<N> &val, unsigned char precision) {
            return int(val.toFixed(precision));
        }

        template<typename T>
        static T pi() {
            return T{float(PI)};
        }

        template<unsigned N>
        static
        Q<N> sin(const Q<N> &radians) {
            const float radians_f = radians.toFloat();
            return Q<N>(std::sinf(radians_f));
        }

        template<unsigned N>
        static
        Q<N> cos(const Q<N> &radians) {
            const auto half_pi = Q<N>(HALF_PI);
            return sin(radians + half_pi);
        }

        template<unsigned N>
        static
        Q<N> tan(const Q<N> &radians) {
            const float radians_f = radians.toFloat();
            return Q<N>(std::tanf(radians_f));
        }

        template<unsigned N>
        static
        Q<N> deg_to_rad(const Q<N> &degrees) {
            using q = Q<N>;
            return (degrees * math::pi<q>()) / q(180);
        }

        template<unsigned N>
        static
        Q<N> mod(const Q<N> &numer, const Q<N> &denom) {
            return numer % denom;
        }

        template<unsigned N>
        static Q<N> sqrt(const Q<N> &val) {
            return val.sqrt();
        }

    }
}