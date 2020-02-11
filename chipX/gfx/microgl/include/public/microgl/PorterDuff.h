#pragma once

#include <microgl/color.h>
#include "crpt.h"

using namespace microgl::color;

namespace microgl {
    namespace porterduff {
#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)
#define MAX_VAL_BITS2(bits) ((1<<(bits)) - 1)
#define absf_(v) (v)<0?-(v):v

        template<typename IMPL>
        class PorterDuffBase : public crpt<IMPL> {
        public:

            inline static void
            composite(const color_t &b, const color_t &s, color_t &output, const unsigned int alpha_bits) {
                IMPL::composite(b, s, output, alpha_bits);
            }

            inline static void composite(const color_f_t &b, const color_f_t &s, color_f_t &output) {
                IMPL::composite(b, s, output);
            }

        protected:
            inline static const char *type() {
                return IMPL::type();
            }

            // a0 = αs x Fa + αb x Fb
            // co = αs x Fa x Cs + αb x Fb x Cb
            // according to PDF spec, page 322, if we use source-over
            // result is NOT alpha pre-multiplied color
            inline static void internal_porter_duff(float Fa, float Fb,
                                                    const color_f_t &b,
                                                    const color_f_t &s,
                                                    color_f_t &output,
                                                    bool multiplied_alpha_result = false) {
                float as = s.a;
                float ab = b.a;

                float as_Fa = as * Fa;
                float ab_Fb = ab * Fb;

                output.a = as_Fa + ab_Fb;

                if (absf_(output.a - 0.0) > 0.000001f) {
                    // unmultiply alpha since the Porter-Duff equation results
                    // in pre-multiplied alpha colors
                    //result.rgb = (as * Fa * Cs + ab * Fb * Cb) / result.a;

                    output.r = (as_Fa * s.r + ab_Fb * b.r);
                    output.g = (as_Fa * s.g + ab_Fb * b.g);
                    output.b = (as_Fa * s.b + ab_Fb * b.b);

                    if (!multiplied_alpha_result && absf_(output.a - 1.0f) > 0.00001) {
                        output.r /= output.a;
                        output.g /= output.a;
                        output.b /= output.a;
                    }
                }

            }

            // a0 = αs x Fa + αb x Fb
            // co = αs x Fa x Cs + αb x Fb x Cb
            // according to PDF spec, page 322, if we use source-over
            // result is NOT alpha pre-multiplied color
            inline static void internal_porter_duff(int Fa, int Fb,
                                                    const color_t &b,
                                                    const color_t &s,
                                                    color_t &output,
                                                    const unsigned int alpha_bits,
                                                    bool multiplied_alpha_result = false) {

                unsigned int as = s.a; // 128
                unsigned int ab = b.a;  // 255

                const unsigned int alpha_bits_double = alpha_bits << 1;

                // these are less than 16 bits
                unsigned int as_Fa = as * Fa; // 128 * 255
                unsigned int ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
                unsigned int combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits
                unsigned int r_channel, g_channel, b_channel;

                // cheaper alpha with bit shifting
                output.a = combined >> alpha_bits;

                // this is multiplied alpha channels
                r_channel = as_Fa * s.r + ab_Fb * b.r;
                g_channel = as_Fa * s.g + ab_Fb * b.g;
                b_channel = as_Fa * s.b + ab_Fb * b.b;

                /*
                // cheap optimization for opaque backdrops, but the branching seems expensive for the compiler
                const unsigned int double_alpha_max_range = 1u<<(alpha_bits_double);
                if(combined==double_alpha_max_range - (1<<(alpha_bits+1)) + 1) {

                    output.r = r_channel>>alpha_bits_double;
                    output.g = g_channel>>alpha_bits_double;
                    output.b = b_channel>>alpha_bits_double;

                    return;
                }
                 */

                // if desired result should be un multiplied
                if (!multiplied_alpha_result) {
                    if (combined) {
                        output.r = r_channel / combined;
                        output.g = g_channel / combined;
                        output.b = b_channel / combined;
                    }
                } else {
                    // keep multiplied result, but of course transform to
                    // the correct bit space
                    output.r = r_channel >> alpha_bits_double;
                    output.g = g_channel >> alpha_bits_double;
                    output.b = b_channel >> alpha_bits_double;
                }

            }

        };

    }
}