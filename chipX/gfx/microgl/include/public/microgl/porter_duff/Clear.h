#pragma once

#include <microgl/PorterDuff.h>

namespace porterduff {

    class Clear : public PorterDuffBase<Clear> {
    public:
        inline static const char * type() {
            return "Clear";
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {

            internal_porter_duff(0, 0, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(0.0, 0.0, b, s, output);
        }

    };

}
