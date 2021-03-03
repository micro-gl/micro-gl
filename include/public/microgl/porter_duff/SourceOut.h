#pragma once

#include <microgl/porter_duff_base.h>

namespace microgl {
    namespace porterduff {

        template <bool fast=true>
        class SourceOut : public porter_duff_base<SourceOut<fast>> {
        public:

            template <uint8_t bits, bool multiplied_alpha_result=true, bool use_FPU=true>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output) {
                constexpr unsigned int max_val =(1<<bits)-1;
                apply_porter_duff<bits, fast, multiplied_alpha_result, use_FPU>(max_val - b.a, 0,
                                                                          b, s, output);
            }

        };

    }
}