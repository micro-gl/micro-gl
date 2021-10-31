#pragma once

#include "BPP_RGB.h"

namespace microgl {
    namespace coder {

        template <unsigned char R, unsigned char G, unsigned char B>
        using BPP_4_RGB = BPP_RGB<4, R,G,B>;
    }
}