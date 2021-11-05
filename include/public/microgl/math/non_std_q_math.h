#pragma once

#include <microgl/math/Q.h>
#include <microgl/math/base_math.h>

namespace microgl {
    namespace math {

        template<unsigned N, typename integer, typename inter_integer, char s>
        inline int to_fixed(const Q<N, integer, inter_integer, s> &val, unsigned char precision) {
            return int(val.toFixed(precision));
        }

        template<unsigned N, typename integer, typename inter_integer, char s> static
        Q<N, integer, inter_integer, s> sin(const Q<N, integer, inter_integer, s> &radians) {
            using q = Q<N, integer, inter_integer, s>;
            return microgl::math::sin_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer, typename inter_integer, char s> static
        Q<N, integer, inter_integer, s> cos(const Q<N, integer, inter_integer, s> &radians) {
            using q = Q<N, integer, inter_integer, s>;
            return microgl::math::cos_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer, typename inter_integer, char s>
        static Q<N, integer, inter_integer, s> tan(const Q<N, integer, inter_integer, s> &radians) {
            using q = Q<N, integer>;
            return microgl::math::tan_bhaskara_cpu<q>(radians);
        }

        template<unsigned N, typename integer, typename inter_integer, char s> static
        Q<N, integer, inter_integer, s> mod(const Q<N, integer, inter_integer, s> &numer, const Q<N, integer, inter_integer, s> &denom) {
            return numer % denom;
        }

        template<unsigned N, typename integer, typename inter_integer, char s>
        static Q<N, integer, inter_integer, s> sqrt(const Q<N, integer, inter_integer, s> &val) {
            return val.sqrt();
        }
    }
}