#pragma once

#include <microgl/porter_duff_base.h>

namespace microgl {
    namespace porterduff {

        template <bool fast=true>
        class None : public porter_duff_base<None<fast>> {
        public:

            template <uint8_t bits, bool multiplied_alpha_result=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output) {
                output.r = s.r;
                output.g = s.g;
                output.b = s.b;
                output.a = s.a;
                if (multiplied_alpha_result) {
                    if(fast) {
                        output.r = (uint(output.r) * output.a) >> bits;
                        output.g = (uint(output.g) * output.a) >> bits;
                        output.b = (uint(output.b) * output.a) >> bits;
                    } else {
                        output.r = mc<bits>(output.r, output.a);
                        output.g = mc<bits>(output.g, output.a);
                        output.b = mc<bits>(output.b, output.a);
                    }

                }

            }

        };

    }
}