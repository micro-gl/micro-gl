#pragma once

#include <microgl/porter_duff_base.h>

namespace microgl {
    namespace porterduff {

        struct FastSourceOverOnOpaque {

            template <uint8_t bits, bool multiplied_alpha_result=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output) {
                // this is to avoid branching as well
                // here output alpha is always = 1.0, therefore we can optimize,
                // there is no difference between pre to un multiplied result.
                constexpr uint16_t max_val = uint16_t(1<<bits)-1;
                const uint16_t comp = max_val - s.a;
//                unsigned int comp = s.a ^max_val; // flipping bits equals (max_val - s.a)
                output.r = (s.a * s.r + comp * b.r) >> bits;
                output.g = (s.a * s.g + comp * b.g) >> bits;
                output.b = (s.a * s.b + comp * b.b) >> bits;
                output.a = max_val;
            }

        };

    }
}