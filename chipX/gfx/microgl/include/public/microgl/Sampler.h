#pragma once

#include <microgl/vec2.h>
#include "crpt.h"
#include <microgl/Bitmap.h>
#include <microgl/Fixed.h>

namespace sampler {
#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)

    template<typename IMPL>
    class SamplerBase : public crpt<IMPL> {
    public:

        template<typename P, typename Coder>
        inline static void sample(const Bitmap<P, Coder> &bmp, 
                                  const fixed_signed u, const fixed_signed v, 
                                  const uint8_t bits, color_t & output) {
            IMPL::sample(bmp, u, v, bits, output);
        }

    protected:
        inline static const char * type() {
            return IMPL::type();
        }

    };

}
