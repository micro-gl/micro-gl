#pragma once

#include "Types.h"
#include <math.h>

enum BlendMode {
    PassThrough = -2,
    None = -1,

    Normal = 0,
    Dissolve = 1,

    Darken = 2,
    Multiply = 3,
    ColorBurn = 4,
    LinearBurn = 5,
    DarkerColor = 6,

    Lighten = 7,
    Screen = 8,
    ColorDodge = 9,
    LinearDodge = 10,
    LighterColor = 11,

    Overlay = 12,
    SoftLight = 13,
    HardLight = 14,
    VividLight = 15,
    LinearLight = 16,
    PinLight = 17,
    HardMix = 18,

    Difference = 19,
    Exclusion = 20,
    Subtract = 21,
    Divide = 22,

    Hue = 23,
    Saturation = 24,
    Color = 25,
    Luminosity = 26,
};

color_f_t blend_mode_apply(BlendMode mode, const color_f_t &b, const color_f_t &s);