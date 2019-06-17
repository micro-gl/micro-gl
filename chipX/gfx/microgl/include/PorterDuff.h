#pragma once

#include "Types.h"
#include <math.h>

enum PorterDuff {
    Clear = 0,
    Copy = 1,
    Destination = 2,
    SourceOver = 3,
    DestinationOver = 4,
    SourceIn = 5,
    DestinationIn = 6,
    SourceOut = 7,
    DestinationOut = 8,
    SourceAtop = 9,
    DestinationAtop = 10,
    XOR = 11,
    Lighter = 12,
    Source = 13,
};

color_f_t porter_duff_apply(PorterDuff mode, const color_f_t & b, const color_f_t & s);