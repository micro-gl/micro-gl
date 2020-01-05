#pragma once

#include <microgl/color.h>
#include "crpt.h"

using namespace microgl::color;

namespace porterduff {
#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)
#define MAX_VAL_BITS2(bits) ((1<<(bits)) - 1)
#define absf_(v) (v)<0?-(v):v

    template<typename IMPL>
    class PorterDuffBase : public crpt<IMPL> {
    public:

        inline static void composite(const color_t & b, const color_t & s, color_t &output, const unsigned int alpha_bits) {
            IMPL::composite(b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b, const color_f_t & s, color_f_t &output) {
            IMPL::composite(b, s, output);
        }

    protected:
        inline static const char * type() {
            return IMPL::type();
        }

        // a0 = αs x Fa + αb x Fb
        // co = αs x Fa x Cs + αb x Fb x Cb
        // according to PDF spec, page 322, if we use source-over
        // result is NOT alpha pre-multiplied color
        inline static void internal_porter_duff(float Fa, float Fb,
                                                const color_f_t & b,
                                                const color_f_t & s,
                                                color_f_t &output) {
            float as = s.a;
            float ab = b.a;

            float as_Fa = as * Fa;
            float ab_Fb = ab * Fb;

            output.a = as_Fa + ab_Fb;

            if(absf_(output.a - 0.0) > 0.000001f) {
                // unmultiply alpha since the Porter-Duff equation results
                // in pre-multiplied alpha colors
                //result.rgb = (as * Fa * Cs + ab * Fb * Cb) / result.a;

                output.r = (as_Fa * s.r + ab_Fb * b.r);
                output.g = (as_Fa * s.g + ab_Fb * b.g);
                output.b = (as_Fa * s.b + ab_Fb * b.b);

                if(absf_(output.a - 1.0f) > 0.00001) {
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
                    const color_t & b,
                    const color_t & s,
                    color_t &output,
                    const unsigned int alpha_bits) {

            unsigned int as = s.a; // 128
            unsigned int ab = b.a;  // 255

            unsigned int max_alpha_range = (1<<alpha_bits);
            unsigned int max_alpha_value = max_alpha_range - 1;
            unsigned int double_alpha_max_range = (1<<(alpha_bits+alpha_bits));


            // these are less than 2^16
            unsigned int as_Fa = as * Fa; // 128 * 255
            unsigned int ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
            unsigned int combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits

            // cheaper alpha with bit shifting
            output.a = combined >> alpha_bits;

            // unmultiplied-alpha result

            // cheap optimization for opaque backdrops
            if(combined==double_alpha_max_range - (1<<(alpha_bits+1)) + 1) {
                const unsigned int alpha_bits_double = alpha_bits<<1;

                output.r = (as_Fa * s.r + ab_Fb * b.r) >> (alpha_bits_double); // inner expression is at most 26 bits, so no overflow
                output.g = (as_Fa * s.g + ab_Fb * b.g) >> (alpha_bits_double);
                output.b = (as_Fa * s.b + ab_Fb * b.b) >> (alpha_bits_double);
                return;
            }

            if(combined) {
                output.r = (as_Fa * s.r + ab_Fb * b.r)/ combined;
                output.g = (as_Fa * s.g + ab_Fb * b.g)/ combined;
                output.b = (as_Fa * s.b + ab_Fb * b.b)/ combined;
            }

            // multiplied-alpha result

//            if(combined) {
//                const unsigned int alpha_bits_double = alpha_bits + alpha_bits;
//                output.r = (as_Fa * s.r + ab_Fb * b.r) >> alpha_bits_double; // maybe ????
//                output.g = (as_Fa * s.g + ab_Fb * b.g) >> alpha_bits_double;
//                output.b = (as_Fa * s.b + ab_Fb * b.b) >> alpha_bits_double;
//            }

        }

    };

}