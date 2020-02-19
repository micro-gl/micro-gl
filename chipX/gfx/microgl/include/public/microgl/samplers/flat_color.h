#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>

namespace microgl {
    namespace sampling {

        class flat_color : public sampler<flat_color> {
            using base= sampler<flat_color>;
            using l64= long long;

        public:
            explicit flat_color()= default;
            explicit flat_color(const color_t & $color) : color{$color} {}

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                output=color;
            }

            color_t color;
        };

    }
}