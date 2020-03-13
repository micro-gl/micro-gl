#pragma once

#include <microgl/color.h>
#include <microgl/crpt.h>

namespace microgl {
    namespace blendmode {
        using namespace microgl::color;
        using bits = unsigned char;
        using uint = unsigned int;
        using cuint = const unsigned int;

#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)


        template<typename IMPL>
        class BlendModeBase : public crpt<IMPL> {
        protected:
        public:

            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output,
                                     const uint8_t r_bits,
                                     const uint8_t g_bits,
                                     const uint8_t b_bits) {

                IMPL::blend(b, s, output, r_bits, g_bits, b_bits);
            }

//            static inline void blend(const color_f_t &b,
//                                     const color_f_t &s,
//                                     color_f_t &output) {
//
//                IMPL::blend(b, s, output);
//            }

            static inline const char *type() {
                return IMPL::type();
            }

        };

    }

}