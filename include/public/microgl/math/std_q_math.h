#pragma once

#include <microgl/Q.h>
#include <microgl/math/base_math.h>
#include <cmath>

namespace microgl {
    namespace math {

        template<unsigned N, typename integer>
        inline int to_fixed(const Q<N, integer> &val, unsigned char precision) {
            return int(val.toFixed(precision));
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
            using q = Q<N, integer>;
            return microgl::math::sin<N, integer>(radians + microgl::math::half_pi<q>());
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> tan(const Q<N, integer> &radians) {
            using q = Q<N, integer>;
            const auto sin_ = microgl::math::sin<N, integer>(radians);
            const auto cos_ = microgl::math::cos<N, integer>(radians);
            return sin_/cos_;
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