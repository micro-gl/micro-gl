#pragma once

#include <microgl/pixel_coders/BPP_RGB.h>

namespace microgl {
    namespace coder {
        template <channel R, channel G, channel B>
        class BPP_8_RGB : public BPP_RGB<8, R,G,B> {
        private:
            using base=BPP_RGB<8, R,G,B>;
        public:
            using base::decode;
            using base::encode;

            BPP_8_RGB() : base{} {}
        };
    }
}