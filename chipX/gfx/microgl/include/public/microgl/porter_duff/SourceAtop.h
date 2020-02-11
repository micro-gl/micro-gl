#pragma once

#include <microgl/PorterDuff.h>

namespace microgl {
    namespace porterduff {

        class SourceAtop : public PorterDuffBase<SourceAtop> {
        public:
            inline static const char *type() {
                return "SourceAtop";
            }

            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output,
                                         const unsigned int alpha_bits,
                                         bool multiplied_alpha_result = false) {
                unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

                internal_porter_duff(b.a, max_val - s.a, b, s, output, alpha_bits, multiplied_alpha_result);
            }

            inline static void composite(const color_f_t &b,
                                         const color_f_t &s,
                                         color_f_t &output,
                                         bool multiplied_alpha_result = false) {
                internal_porter_duff(float(b.a), 1.0 - float(s.a), b, s, output, multiplied_alpha_result);
            }

        };

    }
}