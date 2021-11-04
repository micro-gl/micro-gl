#pragma once

#include <microgl/Q.h>
#include <microgl/math/base_math.h>

namespace microgl {
    namespace math {

        template<unsigned N, typename integer, typename inter_integer>
        inline int to_fixed(const Q<N, integer, inter_integer> &val, unsigned char precision) {
            return int(val.toFixed(precision));
        }

        template<unsigned N, typename integer, typename inter_integer> static
        Q<N, integer, inter_integer> sin(const Q<N, integer, inter_integer> &radians) {
            using q = Q<N, integer, inter_integer>;
            return microgl::math::sin_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer, typename inter_integer> static
        Q<N, integer, inter_integer> cos(const Q<N, integer, inter_integer> &radians) {
            using q = Q<N, integer, inter_integer>;
            return microgl::math::cos_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer, typename inter_integer>
        static Q<N, integer, inter_integer> tan(const Q<N, integer, inter_integer> &radians) {
            using q = Q<N, integer>;
            return microgl::math::tan_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer, typename inter_integer> static
        Q<N, integer, inter_integer> mod(const Q<N, integer, inter_integer> &numer, const Q<N, integer, inter_integer> &denom) {
            return numer % denom;
        }

        template<unsigned N, typename integer, typename inter_integer>
        static Q<N, integer> sqrt(const Q<N, integer, inter_integer> &val) {
            using q = Q<N, integer, inter_integer>;
            return microgl::math::sqrt_cpu(val, q(1)/q(1u<<N));
//            return val.sqrt();
        }
    }
}