#pragma once

#include <microgl/pixel_coders/BPP_RGB.h>

namespace microgl {
    namespace coder {
        template <channel R, channel G, channel B>
        class BPP_4_RGB : public BPP_RGB<4, R,G,B> {
        private:
            using base=BPP_RGB<4, R,G,B>;
        public:
            using base::decode;
            using base::encode;

            BPP_4_RGB() : base{} {}
        };
    }
}