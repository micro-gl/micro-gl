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
            return number{float(PI)};
        }

        template<typename number>
        static number half_pi() {
            return number{float(PI/2.0f)};
        }

        template<typename number>
        inline
        number deg_to_rad(const number &degrees) {
            return ((degrees*number(microgl::math::pi<number>()))/number{180});
        }

    };
}