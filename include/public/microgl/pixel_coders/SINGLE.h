#pragma once

#include "RGBA_PACKED.h"

namespace microgl {
    namespace coder {

        template<uint8_t bits>
        using R = RGBA_PACKED<bits,0,0,0>;

        template<uint8_t bits>
        using G = RGBA_PACKED<0,bits,0,0>;

        template<uint8_t bits>
        using B = RGBA_PACKED<0,0,bits,0>;

        template<uint8_t bits>
        using A = RGBA_PACKED<0,0,0,bits>;
    }
}