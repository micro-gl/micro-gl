#pragma once

#include "crpt.h"
#include <microgl/Bitmap.h>
#include <microgl/math.h>
#include <microgl/precision.h>

namespace microgl {
    namespace sampler {

        template<typename IMPL>
        class SamplerBase : public crpt<IMPL> {
        public:

            template<typename P, typename Coder>
            inline static void sample(const Bitmap<P, Coder> &bmp,
                                      const int u, const int v,
                                      const uint8_t bits, color_t &output) {
                IMPL::sample(bmp, u, v, bits, output);
            }

            template<typename number, typename P, typename Coder>
            inline static void sampleUnit(const Bitmap<P, Coder> &bmp,
                                          const number &u, const number &v,
                                          const enum microgl::precision &p,
                                          color_t &output) {
                const auto bits = int(p);
                const int u_fixed = math::to_fixed(u * (bmp.width() - 1), bits);
                const int v_fixed = math::to_fixed(v * (bmp.height() - 1), bits);
                IMPL::sample(bmp, u_fixed, v_fixed, bits, output);
            }

        protected:
            inline static const char *type() {
                return IMPL::type();
            }

        };

    }
}