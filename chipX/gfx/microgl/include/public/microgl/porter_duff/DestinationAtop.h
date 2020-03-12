#pragma once

#include <microgl/PorterDuff.h>

namespace microgl {
    namespace porterduff {

        template <bool fast=true>
        class DestinationAtop : public PorterDuffBase<DestinationAtop<fast>> {
        public:
            inline static const char *type() {
                return "DestinationAtop";
            }

            template <bool multiplied_alpha_result=true, bool use_FPU=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output,
                                         const unsigned int alpha_bits) {
                const unsigned int max_val =(1<<alpha_bits)-1;
                apply_porter_duff<fast, multiplied_alpha_result, use_FPU>(max_val - b.a, s.a,
                                                                          b, s, output, alpha_bits);
            }

        };

    }
}