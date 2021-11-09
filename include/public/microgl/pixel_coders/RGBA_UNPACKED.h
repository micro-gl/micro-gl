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

#include "../color.h"
#include "../stdint.h"
#include "../traits.h"

namespace microgl {
    namespace coder {

        template<unsigned N>
        struct array {
            uint8_t data[N];
        };

        /**
         * create a pixel coder, that unpacks color channels at the smallest possible array.
         * this coder calculates all of this config at compile-time.
         * pixel type is uint8_t array, that calculates it's size at compile time.
         *
         * @tparam r red channel bits <=8
         * @tparam g green channel bits <=8
         * @tparam b blue channel bits <=8
         * @tparam a alpha channel bits <=8
         * @tparam delete_zero_channel this will make the pixel array smaller
         */
        template<uint8_t r, uint8_t g, uint8_t b, uint8_t a, bool delete_zero_channel=true>
        class RGBA_UNPACKED {
        public:
            static constexpr uint8_t count = delete_zero_channel ?
                                             ((r==0?0:1)+(g==0?0:1)+(b==0?0:1)+(a==0?0:1))
                                             : 4;
            using rgba = rgba_t<r,g,b,a>;
            using pixel = array<count> ;

        private:
            // compile time
            static constexpr bool del_r = delete_zero_channel && r==0;
            static constexpr bool del_g = delete_zero_channel && g==0;
            static constexpr bool del_b = delete_zero_channel && b==0;
            static constexpr bool del_a = delete_zero_channel && a==0;
            static constexpr int8_t r_i = !del_r ? 0 : -1;
            static constexpr int8_t g_i = !del_g ? r_i+1 : r_i;
            static constexpr int8_t b_i = !del_b ? g_i+1 : g_i;
            static constexpr int8_t a_i = !del_a ? b_i+1 : b_i;

        public:
            inline void static
            encode(const color_t &input, pixel &output) {
                if(!del_r) output.data[r_i] = input.r;
                if(!del_g) output.data[g_i] = input.g;
                if(!del_b) output.data[b_i] = input.b;
                if(!del_a) output.data[a_i] = input.a;
            }

            inline void static
            decode(const pixel &input, color_t &output) {
                if(!del_r) output.r = (input.data[r_i]);
                if(!del_g) output.g = (input.data[g_i]);
                if(!del_b) output.b = (input.data[b_i]);
                if(!del_a) output.a = (input.data[a_i]);
            };

        };

    }

}
