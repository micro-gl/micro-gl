#pragma once

#include "../math.h"

namespace microgl {
    namespace functions {

        template<typename number>
        static number length(const number &p_x, const number & p_y) {
            return microgl::math::sqrt(p_x*p_x + p_y*p_y);
        }

        template<typename number>
        static number distance(number p0_x, number p0_y, number p1_x, number p1_y) {
            auto dx= p0_x-p1_x;
            auto dy= p0_y-p1_y;
            return length(dx, dy);
        }
    }
}
