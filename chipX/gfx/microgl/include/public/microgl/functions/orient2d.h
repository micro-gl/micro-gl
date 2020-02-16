#pragma once

namespace microgl {
    namespace functions {
        using l64=long long;

        l64 orient2d(const int &a_x, const int &a_y, const int &b_x, const int &b_y,
                     const int &c_x, const int &c_y, unsigned char precision) {
            return (l64(b_x-a_x)*(c_y-a_y)>>precision) - (l64(b_y-a_y)*(c_x-a_x)>>precision);
        }

        template<typename number>
        inline number orient2d(const number &a_x, const number &a_y,
                               const number &b_x, const number &b_y,
                               const number &c_x, const number &c_y,
                               unsigned char precision) {
            return (b_x-a_x)*(c_y-a_y) - (b_y-a_y)*(c_x-a_x);
        }

    }
}
