#pragma once

#include <microgl/PorterDuff.h>

namespace porterduff {

    class DestinationOver : public PorterDuffBase<DestinationOver> {
    public:
        inline static const char * type() {
            return "DestinationOver";
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val-b.a, max_val, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0 - float(b.a), 1.0, b, s, output);
        }

    };

}
