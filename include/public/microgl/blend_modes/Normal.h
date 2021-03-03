#pragma once

#include <microgl/BlendMode.h>

namespace microgl {
    namespace blendmode {

        class Normal : public BlendModeBase<Normal> {
        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {
                output.r = s.r;
                output.g = s.g;
                output.b = s.b;
            }

        };

    }
}