#pragma once

#include "../color.h"
#include "../stdint.h"
#include "../micro_gl_traits.h"

namespace microgl {
    namespace blendmode {
        using namespace microgl::color;
        using bits = unsigned char;
        using uint = unsigned int;
        using cuint = const unsigned int;
        using cint = const int;

        template<typename impl>
        class blend_mode_base : public microgl::traits::crpt<impl> {
        protected:
        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                impl::template blend<R, G, B>(b, s, output);
            }

        };

    }

}