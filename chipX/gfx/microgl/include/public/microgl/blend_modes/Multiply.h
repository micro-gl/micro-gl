#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        template <bool fast=true>
        class Multiply : public BlendModeBase<Multiply<fast>> {
        public:

            template <bool use_FPU=true>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                using cuint=unsigned int;
                cuint r_= cuint(b.r) * s.r;
                cuint g_= cuint(b.g) * s.g;
                cuint b_= cuint(b.b) * s.b;
                if(fast) {
                    output.r = r_ >> r_bits;
                    output.g = g_ >> g_bits;
                    output.b = b_ >> b_bits;
                } else {
                    output.r = use_FPU ? (float(r_)/ float((1<<r_bits)-1)) : r_ / ((1<<r_bits)-1);
                    output.g = use_FPU ? (float(g_)/ float((1<<g_bits)-1)) : g_ / ((1<<g_bits)-1);
                    output.b = use_FPU ? (float(b_)/ float((1<<b_bits)-1)) : b_ / ((1<<b_bits)-1);
                }

            }

            static inline const char *type() {
                return "Multiply";
            }

        };

    }
}