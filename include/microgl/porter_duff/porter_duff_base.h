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

namespace microgl {
    namespace porterduff {

        using cuint = const unsigned int;
        using uint = unsigned int;
        using uint8_t = microgl::ints::uint8_t;
        // a0 = αs x Fa + αb x Fb
        // co = αs x Fa x Cs + αb x Fb x Cb
        // according to PDF spec, page 322, if we use source-over
        // result is NOT alpha pre-multiplied color

        /**
         * this is a stable version
         * @tparam bits
         * @tparam multiplied_alpha_result
         * @tparam use_fpu
         * @param Fa
         * @param Fb
         * @param b
         * @param s
         * @param output
         */
        template <uint8_t bits, bool multiplied_alpha_result, bool use_fpu>
        void apply_porter_duff_stable(uint8_t Fa, uint8_t Fb,
                               const color_t &b,
                               const color_t &s,
                               color_t &output) {
            auto as = s.a; // 128
            auto ab = b.a; // 255

            auto as_Fa = microgl::mc<bits>(as, Fa); // 128 * 255
            auto ab_Fb = mc<bits>(ab, Fb); // 255 * (255 - 128) = 255 * 127

            output.r = microgl::mc<bits>(as_Fa, s.r) + microgl::mc<bits>(ab_Fb, b.r);
            output.g = microgl::mc<bits>(as_Fa, s.g) + microgl::mc<bits>(ab_Fb, b.g);
            output.b = microgl::mc<bits>(as_Fa, s.b) + microgl::mc<bits>(ab_Fb, b.b);
            output.a = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be (bits+1)

            // if desired result should be un multiplied
            if (!multiplied_alpha_result) {
                output.r = use_fpu ? (cuint) (float(output.r) / float(output.a)) : (output.r / output.a);
                output.g = use_fpu ? (cuint) (float(output.g) / float(output.a)) : (output.g / output.a);
                output.b = use_fpu ? (cuint) (float(output.b) / float(output.a)) : (output.b / output.a);
            }

        }

        /**
         * this is a less stable version, but is much faster
         * @tparam bits
         * @tparam fast
         * @tparam multiplied_alpha_result
         * @tparam use_fpu
         * @param Fa
         * @param Fb
         * @param b
         * @param s
         * @param output
         */
        template <uint8_t bits, bool fast, bool multiplied_alpha_result, bool use_fpu>
        void apply_porter_duff(int Fa, int Fb,
                               const color_t &b,
                               const color_t &s,
                               color_t &output) {
            cuint as = s.a; // 128
            cuint ab = b.a;  // 255
            constexpr cuint max = (1<<bits)-1;
            // these are less than 16 bits
            cuint as_Fa = as * Fa; // 128 * 255
            cuint ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
            cuint combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits
            if(fast)
                output.a =combined>>bits;
            else {
                output.a= use_fpu ? cuint(float(combined)/float(max)) : (combined/max);
            }
            // this is multiplied alpha channels
            // these might get to 24 bits
            cuint r_channel = as_Fa * s.r + ab_Fb * b.r;
            cuint g_channel = as_Fa * s.g + ab_Fb * b.g;
            cuint b_channel = as_Fa * s.b + ab_Fb * b.b;
            // if desired result should be un multiplied
            if (multiplied_alpha_result) {
                if(fast) {
                    constexpr cuint alpha_bits_double = bits << 1;
                    output.r = r_channel >> alpha_bits_double;
                    output.g = g_channel >> alpha_bits_double;
                    output.b = b_channel >> alpha_bits_double;

                } else {
                    cuint max_double = max*max;
                    // no need for FPU here, the compiler uses a trick for the
                    // special max_double division
                    output.r = (cuint)((r_channel)/(max_double));
                    output.g = (cuint)((g_channel)/(max_double));
                    output.b = (cuint)((b_channel)/(max_double));
                }
            } else {
                if (combined) {
                    output.r = use_fpu ? (cuint)(float(r_channel)/float(combined)) : (r_channel/combined);
                    output.g = use_fpu ? (cuint)(float(g_channel)/float(combined)) : (g_channel/combined);
                    output.b = use_fpu ? (cuint)(float(b_channel)/float(combined)) : (b_channel/combined);
                }
            }
        }

        template<typename impl>
        class porter_duff_base {
        public:

            template <uint8_t bits, bool multiplied_alpha_result=true, bool use_FPU=false>
            inline static void
            composite(const color_t &b, const color_t &s, color_t &output) {

                impl::template composite<bits, multiplied_alpha_result, use_FPU>(b, s, output);
            }

        };

    }
}