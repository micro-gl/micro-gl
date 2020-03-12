#pragma once

#include <microgl/PorterDuff.h>

namespace microgl {
    namespace porterduff {

        template <bool fast=true>//, bool multiplied_alpha_result=true>
        class SourceOver2 : public PorterDuffBase<SourceOver2<fast>> {
        public:
            inline static const char *type() {
                return "SourceOver";
            }

            template <bool multiplied_alpha_result=true, bool use_FPU=false>
            inline static void composite(const color_t &b,
                                         const color_t &s,
                                         color_t &output,
                                         const unsigned int alpha_bits) {
                const unsigned int max_val =(1<<alpha_bits)-1;
                apply_porter_duff<fast, multiplied_alpha_result, use_FPU>(max_val, max_val - s.a, b, s,
                        output, alpha_bits);
//
//                    apply_porter_duff_fast(max_val, max_val - s.a, b, s,
//                                           output, alpha_bits, multiplied_alpha_result);

//                    apply_porter_duff_accurate(max_val, max_val - s.a, b, s,
//                                           output, alpha_bits, multiplied_alpha_result);
            }

        };

    }
}