#pragma once

#include <microgl/pixel_coders/BPP_RGBA.h>

namespace microgl {
    namespace coder {

        template <typename rgba_>
        using BPP_8_RGBA = BPP_RGBA<8, rgba_>;
    }
}