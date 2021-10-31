#pragma once

#include "BPP_RGB.h"

namespace microgl {
    namespace coder {

        template <unsigned char R, unsigned char G, unsigned char B>
        using BPP_1_RGB = BPP_RGB<1, R,G,B>;
    }
}