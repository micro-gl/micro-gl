#pragma once

#include "porter_duff_base.h"

namespace microgl {
    namespace porterduff {

        template <bool fast=true, bool use_FPU=true>
        struct SourceOut {

            template <uint8_t bits, bool multiplied_alpha_result=true>
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