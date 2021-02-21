#pragma once

#include <microgl/color.h>

using namespace microgl::color;

namespace microgl {
    namespace porterduff {

        using cuint = const unsigned int;
        using uint = unsigned int;
        // a0 = αs x Fa + αb x Fb
        // co = αs x Fa x Cs + αb x Fb x Cb
        // according to PDF spec, page 322, if we use source-over
        // result is NOT alpha pre-multiplied color
        template <bool fast, bool multiplied_alpha_result, bool use_fpu>
        void apply_porter_duff(int Fa, int Fb,
                               const color_t &b,
                               const color_t &s,
                               color_t &output,
                               const unsigned int alpha_bits) {
            cuint as = s.a; // 128
            cuint ab = b.a;  // 255
            cuint max = (1<<alpha_bits)-1;
            // these are less than 16 bits
            cuint as_Fa = as * Fa; // 128 * 255
            cuint ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
            cuint combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits
            if(fast)
                output.a =combined>>alpha_bits;
            else {
                output.a= use_fpu ? cuint(float(combined)/float(max)) : (combined/max);
            }
            // this is multiplied alpha channels
            cuint r_channel = as_Fa * s.r + ab_Fb * b.r;
            cuint g_channel = as_Fa * s.g + ab_Fb * b.g;
            cuint b_channel = as_Fa * s.b + ab_Fb * b.b;
            // if desired result should be un multiplied
            if (multiplied_alpha_result) {
                if(fast) {
                    cuint alpha_bits_double = alpha_bits << 1;
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

        template<typename IMPL>
        class PorterDuffBase {
        public:

            template <bool multiplied_alpha_result=true, bool use_FPU=false>
            inline static void
            composite(const color_t &b, const color_t &s, color_t &output, const unsigned int alpha_bits) {

                IMPL::composite<multiplied_alpha_result, use_FPU>(b, s, output, alpha_bits);
            }

        protected:
            inline static const char *type() {
                return IMPL::type();
            }

        };

    }
}