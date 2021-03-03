#pragma once

#include <microgl/sampler.h>

namespace microgl {
    namespace sampling {

        template<typename rgba_=rgba_t<8,8,8,0>>
        class flat_color : public sampler<rgba_, flat_color<rgba_>> {
            using base= sampler<rgba_, flat_color<rgba_>>;

        public:
            explicit flat_color()= default;
            explicit flat_color(const color_t & $color) : color{$color} {}

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                // we only copy channel values and not bit information
                output.r=color.r;
                output.g=color.g;
                output.b=color.b;
                output.a=color.a;
            }
            color_t color;
        };

    }
}