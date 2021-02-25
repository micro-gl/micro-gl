#pragma once

#include "crpt.h"
#include <microgl/math.h>
#include <microgl/precision.h>

namespace microgl {
    namespace sampling {

        template<channel R_BITS, channel G_BITS, channel B_BITS, channel A_BITS, typename IMPL>
        class sampler : public crpt<IMPL> {
        protected:
            using base_type = sampler;
        public:
            static constexpr uint8_t r = R_BITS;
            static constexpr uint8_t g = G_BITS;
            static constexpr uint8_t b = B_BITS;
            static constexpr uint8_t a = A_BITS;

            sampler()= default;

            inline void sample(const int u, const int v,
                               const uint8_t bits, color_t &output) const {
                this->derived().sample(u, v, bits, output);
            }

            template<typename number>
            inline void sample(const number &u, const number &v,
                               const enum microgl::precision &p, color_t &output) {
                auto bits = static_cast<precision_t>(p);
                const int u_fixed = math::to_fixed(u, bits);
                const int v_fixed = math::to_fixed(v, bits);
                this->derived().sample(u_fixed, v_fixed, bits, output);
            }

        };

    }
}