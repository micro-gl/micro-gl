#pragma once

#include <microgl/PorterDuff.h>

namespace porterduff {

    class DestinationIn : public PorterDuffBase<DestinationIn> {
    public:
        inline static const char * type() {
            return "DestinationIn";
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(0, s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(0.0, float(s.a), b, s, output);
        }

    };

}
