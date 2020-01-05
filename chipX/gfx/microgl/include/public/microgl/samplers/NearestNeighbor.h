#pragma once

#include <microgl/Sampler.h>

namespace sampler {

    class NearestNeighbor : public SamplerBase<NearestNeighbor> {
    public:
        inline static const char * type() {
            return "NearestNeighbor";
        }

        template<typename P, typename Coder>
        inline static void sample(const Bitmap<P, Coder> &bmp,
                                  const int u, const int v,
                                  const uint8_t bits, color_t & output) {

            int h = 0;//(1<<bits) - 1;
            int index_bmp = (bmp.width()*((v+h)>>bits) + ((u+h)>>bits));

            bmp.decode(index_bmp, output);
        }

    };

}