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

#include "../stdint.h"
#include "../color.h"

namespace microgl {
    namespace coder {

        /**
         * create a pixel coder, that packs color channels at the smallest possible 2^N int.
         * this coder calculates all of this config at compile-time, and also supports different
         * layout schemes for packing the channels in any order possible.
         * zero channels will be discarded
         *
         * @tparam r red channel bits <=8
         * @tparam g green channel bits <=8
         * @tparam b blue channel bits <=8
         * @tparam a alpha channel bits <=8
         * @tparam ri position of red channel
         * @tparam gi position of green channel
         * @tparam bi position of blue channel
         * @tparam ai position of alpha channel
         */
        template<unsigned r, unsigned g, unsigned b, unsigned a=0,
                 unsigned ri=0, unsigned gi=1, unsigned bi=2, unsigned ai=3>
        class RGBA_PACKED {
        public:
            using rgba = rgba_t<r,g,b,a>;
            using pixel = microgl::ints::uint_t<r+g+b+a>;
            using u8 = unsigned char;
            static constexpr u8 size_pixel = sizeof (pixel);

        private:
            // compile time
            static constexpr u8 shift_3= 0;
            static constexpr u8 shift_2= ri==3 ? r : (gi==3?g: (bi==3? b : (ai==3 ? a : 0)));
            static constexpr u8 shift_1= shift_2+(ri==2 ? r : (gi==2?g: (bi==2? b : (ai==2 ? a : 0))));
            static constexpr u8 shift_0= shift_1+(ri==1 ? r : (gi==1?g: (bi==1? b : (ai==1 ? a : 0))));
            static constexpr u8 shift[4] = {shift_0, shift_1, shift_2, shift_3};
//            static constexpr u8 shift[4] = {g + b + a, b+a, a, 0};

            static constexpr pixel r_mask_1 = ((1u<<r)-1), r_mask_2 = r_mask_1<<shift[ri];
            static constexpr pixel g_mask_1 = ((1u<<g)-1), g_mask_2 = g_mask_1<<shift[gi];
            static constexpr pixel b_mask_1 = ((1u<<b)-1), b_mask_2 = b_mask_1<<shift[bi];
            static constexpr pixel a_mask_1 = ((1u<<a)-1), a_mask_2 = a_mask_1<<shift[ai];

        public:
            inline void static
            encode(const color_t &input, pixel &output) {
                output = 0;
                if(r!=0) output += ((input.r & r_mask_1)<<shift[ri]);
                if(g!=0) output += ((input.g & g_mask_1)<<shift[gi]);
                if(b!=0) output += ((input.b & b_mask_1)<<shift[bi]);
                if(a!=0) output += ((input.a & a_mask_1)<<shift[ai]);
            }

            inline void static
            decode(const pixel &input, color_t &output) {
                if(r!=0) output.r = (input & r_mask_2) >> shift[ri];
                if(g!=0) output.g = (input & g_mask_2) >> shift[gi];
                if(b!=0) output.b = (input & b_mask_2) >> shift[bi];
                if(a!=0) output.a = (input & a_mask_2) >> shift[ai];
            };

        };

    }

}
