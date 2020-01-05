#pragma once

#include <microgl/PorterDuff.h>

namespace porterduff {

    class SourceOver : public PorterDuffBase<SourceOver> {
    public:
        inline static const char * type() {
            return "SourceOver";
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            // opaque background optimization
//            if(b.a==max_val) {
//                unsigned int comp = max_val - s.a;
//                output.r = (s.a*s.r + comp * b.r) >> alpha_bits;
//                output.g = (s.a*s.g + comp * b.g) >> alpha_bits;
//                output.b = (s.a*s.b + comp * b.b) >> alpha_bits;
//                output.a = max_val;
//
//                return;
//            }

            internal_porter_duff(max_val, max_val - s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0, 1.0f - float(s.a), b, s, output);
        }

    };

}
