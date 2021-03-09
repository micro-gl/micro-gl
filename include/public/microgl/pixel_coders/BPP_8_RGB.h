#pragma once

#include <microgl/pixel_coders/BPP_RGB.h>

namespace microgl {
    namespace coder {

        template <channel R, channel G, channel B>
        using BPP_8_RGB = BPP_RGB<8, R,G,B>;
    }
}