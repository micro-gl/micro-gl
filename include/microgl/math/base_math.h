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

namespace microgl {
    namespace math {

#define PI        3.14159265358979323846264338327950288

        template<typename number>
        number abs(const number &val) {
            return val < 0 ? -val : val;
        }

        template<typename number>
        static number pi() {
            return number(float(PI));
        }

        template<typename number>
        static number half_pi() {
            return number(float(PI/2.0f));
        }

        template<typename number>
        inline
        number deg_to_rad(const number &degrees) {
            return ((degrees*number(microgl::math::pi<number>()))/number(180));
        }

        template<typename number>
        number mod_cpu(const number numer, const number denom) {
            const auto a_div_b = number(int(numer/denom));
            return numer - a_div_b*denom;
        }

        template<typename number>
        number sqrt_cpu(const number val, const number epsilon) {
            int ix = 0;
            number x = val, y = number(1);
            while ((abs<number>(x - y) > epsilon) && (ix++<10)) {
                x = (x + y) / number(2);
                y = val / x;
            }
            return x;
        }

        template<typename number>
        number sin_bhaskara_cpu(number radians) {
            const auto pii = microgl::math::pi<number>();
            const auto pii2 = microgl::math::pi<number>()*number(2);

            auto modd = mod_cpu<number>(radians, pii2);
            modd = modd<0 ? modd+pii2 : (modd>pii2 ? pii2 : modd);
            const number sign = modd<=pii ? number(1) : -number(1);
            modd = modd>=pii ? modd-pii : modd;

            // bhaskara approximation is in [0, pi], so we clip
            const auto x = modd>=pii ? pii : modd;
            auto expr = (number(16)*x*(pii-x))/(number(5)*pii*pii -number(4)*x*(pii-x));
            return expr * sign;
        }

        template<typename number>
        number cos_bhaskara_cpu(number radians) {
            return sin_bhaskara_cpu<number>(radians + microgl::math::half_pi<number>());
        }

        template<typename number>
        number tan_bhaskara_cpu(number radians) {
            return sin_bhaskara_cpu<number>(radians)/cos_bhaskara_cpu<number>(radians);
        }
    };
}