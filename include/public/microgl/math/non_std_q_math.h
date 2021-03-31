#pragma once

#include <microgl/Q.h>
#include <microgl/math/base_math.h>

namespace microgl {
    namespace math {

        template<unsigned N, typename integer>
        inline int to_fixed(const Q<N, integer> &val, unsigned char precision) {
            return int(val.toFixed(precision));
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> sin(const Q<N, integer> &radians) {
            using q = Q<N, integer>;
            return microgl::math::sin_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> cos(const Q<N, integer> &radians) {
            using q = Q<N, integer>;
            return microgl::math::cos_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer>
        static
        Q<N, integer> tan(const Q<N, integer> &radians) {
            using q = Q<N, integer>;
            return microgl::math::tan_bhaskara_cpu<q>(radians);
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