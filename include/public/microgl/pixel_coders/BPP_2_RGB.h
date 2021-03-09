#pragma once

#include <microgl/pixel_coders/BPP_RGB.h>

namespace microgl {
    namespace coder {

        template <channel R, channel G, channel B>
        using BPP_2_RGB = BPP_RGB<2, R,G,B>;
    }
}