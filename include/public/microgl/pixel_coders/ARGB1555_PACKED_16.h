#pragma once

#include <microgl/pixel_coders/RGBA_PACKED.h>

namespace microgl {
    namespace coder {

        using ARGB1555_PACKED_16 = RGBA_PACKED<5,5,5,1, 1,2,3,0>;

    }
}