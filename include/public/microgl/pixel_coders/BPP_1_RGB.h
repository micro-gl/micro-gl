#pragma once

#include <microgl/pixel_coders/BPP_RGB.h>

namespace microgl {
    namespace coder {

        template <channel R, channel G, channel B>
        using BPP_1_RGB = BPP_RGB<1, R,G,B>;
    }
}