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

            template <bool multiplied_alpha_result=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output,
                                         const unsigned int alpha_bits) {
                const unsigned int max_val =(1<<alpha_bits)-1;
                output.r = s.r;
                output.g = s.g;
                output.b = s.b;
                output.a = s.a;
                if (multiplied_alpha_result) {
                    if(fast) {
                        output.r = (uint(output.r) * output.a) >> alpha_bits;
                        output.g = (uint(output.g) * output.a) >> alpha_bits;
                        output.b = (uint(output.b) * output.a) >> alpha_bits;
                    } else {
                        output.r = (uint(output.r) * output.a)/max_val;
                        output.g = (uint(output.g) * output.a)/max_val;
                        output.b = (uint(output.b) * output.a)/max_val;
                    }

                }

            }

        };

    }
}