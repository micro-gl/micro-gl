#pragma once

#include "BPP_RGB.h"

namespace microgl {
    namespace coder {

        template <unsigned char R, unsigned char G, unsigned char B>
        using BPP_8_RGB = BPP_RGB<8, R,G,B>;
    }
}