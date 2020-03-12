#pragma once

#include <microgl/PorterDuff.h>

namespace microgl {
    namespace porterduff {

        template <bool fast=true>
        class None : public PorterDuffBase<None<fast>> {
        public:
            inline static const char *type() {
                return "None";
            }

            template <bool multiplied_alpha_result=true, bool use_FPU=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output,
                                         const unsigned int alpha_bits) {
                const unsigned int max_val =(1<<alpha_bits)-1;
                output.r = s.r;
                output.g = s.g;
                output.b = s.b;
                output.a = s.a;
                // also, if alpha=1.0, no need to multiply
                if (multiplied_alpha_result && (output.a < max_val)) {
                    if(fast) {
                        output.r = (int(output.r) * output.a) >> alpha_bits;
                        output.g = (int(output.g) * output.a) >> alpha_bits;
                        output.b = (int(output.b) * output.a) >> alpha_bits;
                    } else {
                        output.r = (int(output.r) * output.a)/max_val;
                        output.g = (int(output.g) * output.a)/max_val;
                        output.b = (int(output.b) * output.a)/max_val;
                    }

                }

            }

        };

    }
}