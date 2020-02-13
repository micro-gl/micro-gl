#pragma once

#include "crpt.h"
#include <microgl/math.h>
#include <microgl/precision.h>

namespace microgl {
    namespace sampling {

        template<typename IMPL>
        class sampler : public crpt<IMPL> {
        public:

            sampler(bits red_bits,bits green_bits,bits blue_bits,bits alpha_bits) :
                        _red_bits{red_bits}, _green_bits{green_bits}, _blue_bits{blue_bits}, _alpha_bits{alpha_bits} {}

            inline void sample(const int u, const int v,
                               const uint8_t bits, color_t &output) const {
                this->derived().sample(u, v, bits, output);
            }

            template<typename number>
            inline void sampleUnit(const number &u, const number &v,
                                      const enum microgl::precision &p,
                                      color_t &output) {
                const auto bits = int(p);
                const int u_fixed = math::to_fixed(u, bits);
                const int v_fixed = math::to_fixed(v, bits);
                this->derived().sample(u_fixed, v_fixed, bits, output);
            }

            bits red_bits() const { return _red_bits; }
            bits green_bits() const { return _green_bits; }
            bits blue_bits() const { return _blue_bits; }
            bits alpha_bits() const { return _alpha_bits; }

        protected:
            bits _red_bits=8, _green_bits=8, _blue_bits=8, _alpha_bits=8;
        };

    }
}