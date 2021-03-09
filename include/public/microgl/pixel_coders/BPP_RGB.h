#pragma once

#include <microgl/pixel_coder.h>
#include <microgl/lut_bits.h>

namespace microgl {
    namespace coder {

        template <channel BPP, channel R, channel G, channel B>
        struct BPP_RGB {
            using rgba = rgba_t<R, G, B, 0>;
            using pixel = uint8_t;
        private:
            // dynamic look-up tables
            microgl::lut::dynamic_lut_bits<BPP, R> _lut_r;
            microgl::lut::dynamic_lut_bits<BPP, G> _lut_g;
            microgl::lut::dynamic_lut_bits<BPP, B> _lut_b;

        public:
            BPP_RGB() : _lut_r{}, _lut_g{}, _lut_b{} {}

            inline void encode(const color_t &input, uint8_t &output) const {
            }

            inline void decode(const uint8_t &input, color_t &output) const {
                output.r=_lut_r[input];
                output.g=_lut_g[input];
                output.b=_lut_b[input];
            };

        };
    }
}