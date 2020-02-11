#pragma once

#include <microgl/PorterDuff.h>

namespace microgl {
    namespace porterduff {

        class SourceOverOnOpaque : public PorterDuffBase<SourceOverOnOpaque> {
        public:
            inline static const char *type() {
                return "SourceOverOnOpaque";
            }

            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output,
                                         const unsigned int alpha_bits,
                                         bool multiplied_alpha_result = false) {
                // this is to avoid branching as well
                // here output alpha is always = 1.0, therefore we can optimize,
                // there is no difference between pre to un multiplied result.
                unsigned int max_val = MAX_VAL_BITS2(alpha_bits);
//            unsigned int comp = max_val - s.a;
                unsigned int comp = s.a ^max_val; // flipping bits equals (max_val - s.a)

                if (comp) {
                    output.r = (s.a * s.r + comp * b.r) >> alpha_bits;
                    output.g = (s.a * s.g + comp * b.g) >> alpha_bits;
                    output.b = (s.a * s.b + comp * b.b) >> alpha_bits;

                } else {
                    output.r = s.r;
                    output.g = s.g;
                    output.b = s.b;
                }

                output.a = max_val;
            }

            inline static void composite(const color_f_t &b,
                                         const color_f_t &s,
                                         color_f_t &output,
                                         bool multiplied_alpha_result = false) {
                internal_porter_duff(1.0, 1.0 - float(s.a), b, s, output, multiplied_alpha_result);
            }

        };

    }
}