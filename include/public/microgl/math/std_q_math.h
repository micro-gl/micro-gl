#pragma once

#include <microgl/Q.h>
#include <microgl/math/base_math.h>
#include <cmath>

namespace microgl {
    namespace math {

        template<unsigned N, typename integer, typename inter_integer>
        inline int to_fixed(const Q<N, integer, inter_integer> &val, unsigned char precision) {
            return int(val.toFixed(precision));
        }

        template<unsigned N, typename integer, typename inter_integer> static
        Q<N, integer, inter_integer> sin(const Q<N, integer, inter_integer> &radians) {
            const float radians_f = radians.toFloat();
            return Q<N, integer, inter_integer>(std::sinf(radians_f));
        }

        template<unsigned N, typename integer, typename inter_integer> static
        Q<N, integer, inter_integer> cos(const Q<N, integer, inter_integer> &radians) {
            using q = Q<N, integer, inter_integer>;
            return microgl::math::sin<N, integer, inter_integer>(radians + microgl::math::half_pi<q>());
        }

        template<unsigned N, typename integer, typename inter_integer> static
        Q<N, integer, inter_integer> tan(const Q<N, integer, inter_integer> &radians) {
            using q = Q<N, integer, inter_integer>;
            const auto sin_ = microgl::math::sin<N, integer, inter_integer>(radians);
            const auto cos_ = microgl::math::cos<N, integer, inter_integer>(radians);
            return sin_/cos_;
        }

        template<unsigned N, typename integer, typename inter_integer> static
        Q<N, integer, inter_integer> mod(const Q<N, integer, inter_integer> &numer, const Q<N, integer, inter_integer> &denom) {
            return numer % denom;
        }

        template<unsigned N, typename integer, typename inter_integer>
        static Q<N, integer, inter_integer> sqrt(const Q<N, integer, inter_integer> &val) {
            return val.sqrt();
        }
    }
}