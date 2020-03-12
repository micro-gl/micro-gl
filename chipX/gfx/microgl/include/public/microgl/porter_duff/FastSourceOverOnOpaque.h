#pragma once

#include <microgl/PorterDuff.h>

namespace microgl {
    namespace porterduff {

        class FastSourceOverOnOpaque : public PorterDuffBase<FastSourceOverOnOpaque> {
        public:
            inline static const char *type() {
                return "FastSourceOverOnOpaque";
            }

            template <bool multiplied_alpha_result=true, bool use_FPU=false>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output,
                                         const unsigned int alpha_bits) {
                // this is to avoid branching as well
                // here output alpha is always = 1.0, therefore we can optimize,
                // there is no difference between pre to un multiplied result.
                const unsigned int max_val = (1<<alpha_bits)-1;
                const unsigned int comp = max_val - s.a;
//                unsigned int comp = s.a ^max_val; // flipping bits equals (max_val - s.a)
                output.r = (s.a * s.r + comp * b.r) >> alpha_bits;
                output.g = (s.a * s.g + comp * b.g) >> alpha_bits;
                output.b = (s.a * s.b + comp * b.b) >> alpha_bits;
                output.a = max_val;
            }

        };

    }
}