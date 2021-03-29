#pragma once

#include <microgl/Q.h>
#include <cmath>

namespace microgl {
    namespace math {

#define PI        3.14159265358979323846264338327950288
#define HALF_PI   1.5707963268

        template<unsigned N, typename integer>
        inline int to_fixed(const Q<N, integer> &val, unsigned char precision) {
            return int(val.toFixed(precision));
        }

        template<typename T>
        static T pi() {
            return T{float(PI)};
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> sin(const Q<N, integer> &radians) {
            const float radians_f = radians.toFloat();
            return Q<N, integer>(std::sinf(radians_f));
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> cos(const Q<N, integer> &radians) {
            const auto half_pi = Q<N, integer>(HALF_PI);
            return sin(radians + half_pi);
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> tan(const Q<N, integer> &radians) {
            const float radians_f = radians.toFloat();
            return Q<N, integer>(std::tanf(radians_f));
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> deg_to_rad(const Q<N, integer> &degrees) {
            using q = Q<N, integer>;
            return (degrees * math::pi<q>()) / q(180);
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> mod(const Q<N, integer> &numer, const Q<N, integer> &denom) {
            return numer % denom;
        }

        template<unsigned N, typename integer>
        static Q<N, integer> sqrt(const Q<N, integer> &val) {
            return val.sqrt();
        }

    }
}