#pragma once

#include "../rgba_t.h"

namespace microgl {
    namespace sampling {

        /**
         * void sampler is a distinguished sampler in micro-gl, this sampler
         * has 0 bits for each channel and the rasterizers are aware of it and
         * do not draw.
         */
        struct void_sampler {
            using rgba = rgba_t<0,0,0,0>;
            explicit void_sampler()= default;

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
            }
        };
    }
}