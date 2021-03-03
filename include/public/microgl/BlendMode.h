#pragma once

#include <microgl/color.h>
#include <microgl/crpt.h>
#include <microgl/convert_channel.h>

namespace microgl {
    namespace blendmode {
        using namespace microgl::color;
        using bits = unsigned char;
        using uint = unsigned int;
        using cuint = const unsigned int;
        using cint = const int;

#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)


        template<typename impl>
        class BlendModeBase : public crpt<impl> {
        protected:
        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                impl::template blend<R, G, B>(b, s, output);
            }

//            static inline void blend(const color_f_t &b,
//                                     const color_f_t &s,
//                                     color_f_t &output) {
//
//                impl::blend(b, s, output);
//            }

//            static inline const char *type() {
//                return IMPL::type();
//            }

        };

    }

}