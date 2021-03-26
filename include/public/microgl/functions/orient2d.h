#pragma once

namespace microgl {
    namespace functions {

        template <typename coords, typename result>
        result orient2d(const coords &a_x, const coords &a_y, const coords &b_x, const coords &b_y,
                     const coords &c_x, const coords &c_y, unsigned char precision) {
            return ((result(b_x-a_x)*result(c_y-a_y)) - (result(b_y-a_y)*result(c_x-a_x)))>>precision;
        }

        template<typename number>
        inline number
        orient2d(const number &a_x, const number &a_y, const number &b_x, const number &b_y, const number &c_x,
                 const number &c_y) {
            return (b_x-a_x)*(c_y-a_y) - (b_y-a_y)*(c_x-a_x);
        }

    }
}
