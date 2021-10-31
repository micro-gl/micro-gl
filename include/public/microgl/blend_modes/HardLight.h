#pragma once

#include "Overlay.h"

namespace microgl {
    namespace blendmode {

        template <bool fast=true, bool use_FPU=false>
        struct HardLight {

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                Overlay<fast, use_FPU>::template blend<R, G, B>(s, b, output);
            }

        };

    }
}