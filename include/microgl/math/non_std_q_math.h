/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
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