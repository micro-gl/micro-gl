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

#include <microgl/math/base_math.h>

namespace microgl {
    namespace math {
        using u32 = unsigned int;
        using ul64 = unsigned long long;
        using l64 = long long;

        inline int to_fixed(const unsigned &val, unsigned char precision) {
            return (int(val) << precision);
        }
        inline int to_fixed(const int &val, unsigned char precision) {
            const bool isNegative = val < 0;
            int value_abs = microgl::math::abs(int(val)) << precision;
            return isNegative ? -value_abs : value_abs;
        }

        inline u32 sqrt_64(ul64 a_nInput);
        inline u32 sqrt_32(u32 a_nInput);
        inline unsigned int sqrt(unsigned int val) { return sqrt_32(val); }
        inline unsigned int sqrt(int val) { return sqrt_32((unsigned int)val); }
        inline unsigned int sqrt(unsigned long long val) { return sqrt_64(val); }
        inline unsigned int sqrt(long long val) { return sqrt_64((unsigned long long)val); }
        inline int mod(int numer, int denom) { return numer % denom; }
        inline l64 mod(l64 numer, l64 denom) { return numer % denom; }

        inline
        u32 sqrt_64(ul64 a_nInput) {
            ul64 op = a_nInput;
            ul64 res = 0;
            ul64 one = ul64(1u)
                    << 62; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

            // "one" starts at the highest power of four <= than the argument.
            while (one > op) one >>= 2;
            while (one != 0) {
                if (op >= res + one) {
                    op = op - (res + one);
                    res = res + (one << 1);
                }
                res >>= 1;
                one >>= 2;
            }
            return res;
        }

        inline
        u32 sqrt_32(u32 a_nInput) {
            u32 op = a_nInput;
            u32 res = 0;
            u32 one = u32(1u)
                    << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

            // "one" starts at the highest power of four <= than the argument.
            while (one > op) one >>= 2;
            while (one != 0) {
                if (op >= res + one) {
                    op = op - (res + one);
                    res = res + (one << 1);
                }
                res >>= 1;
                one >>= 2;
            }
            return res;
        }
    }
}