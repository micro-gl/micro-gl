#pragma once

#include "crpt.h"
#include <microgl/math.h>
#include <microgl/precision.h>

namespace microgl {
    namespace sampling {

        template<class sampler>
        using sampler_rgba = typename sampler::rgba;

        template<class Sampler, typename number>
        inline void sample(const Sampler & sampler,
                           const number &u, const number &v,
                           const enum microgl::precision &p,
                           color_t &output) {
            auto bits = static_cast<precision_t>(p);
            const int u_fixed = microgl::math::to_fixed(u, bits);
            const int v_fixed = microgl::math::to_fixed(v, bits);
            sampler.sample(u_fixed, v_fixed, bits, output);
        }

        template<typename rgba_, typename impl>
        class sampler : public crpt<impl> {
        protected:
        public:
            using rgba = rgba_;

            sampler()= default;

            inline void sample(const int u, const int v,
                               const uint8_t bits, color_t &output) const {
                this->derived().sample(u, v, bits, output);
            }

            template<typename number>
            inline void sample(const number &u, const number &v,
                               const enum microgl::precision &p,
                               color_t &output) {
                auto bits = static_cast<precision_t>(p);
                const int u_fixed = microgl::math::to_fixed(u, bits);
                const int v_fixed = microgl::math::to_fixed(v, bits);
                this->derived().sample(u_fixed, v_fixed, bits, output);
            }

        };

    }
}