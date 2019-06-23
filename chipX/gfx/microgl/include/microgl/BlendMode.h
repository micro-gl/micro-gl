#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

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

// overlay s
inline float blend_Overlay(float b, float s) {
    return b<0.5?(2.0*b*s):(1.0-2.0*(1.0-b)*(1.0-s));
}

inline color_f_t blend_Overlay(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_Overlay(b.r,s.r),
                     blend_Overlay(b.g,s.g),
                     blend_Overlay(b.b,s.b)};
}

// normal s
inline color_f_t blend_Normal(const color_f_t &b, const color_f_t &s) {
    return s;
}


// multiply s
inline color_f_t blend_Multiply(const color_f_t &b, const color_f_t &s) {
    return color_f_t{   .r=b.r*s.r,
            .g=b.g*s.g,
            .b=b.b*s.b };
}

// divide s
inline float blend_Divide(float b, float s) {
    return (s==0.0) ? 1.0 : (b/s);
}

inline color_f_t blend_Divide(const color_f_t &b, const color_f_t &s) {
    return color_f_t{ blend_Divide(b.r,s.r),
                      blend_Divide(b.g,s.g),
                      blend_Divide(b.b,s.b)};
}

// screen s
inline float blend_Screen(float b, float s) {
    return 1.0-((1.0-b)*(1.0-s));
}

inline color_f_t blend_Screen(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_Screen(b.r,s.r),
                     blend_Screen(b.g,s.g),
                     blend_Screen(b.b,s.b)};
}

// darken s
inline float blend_Darken(float b, float s) {
    return fmin(s,b);
}

inline color_f_t blend_Darken(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_Darken(b.r,s.r),
                     blend_Darken(b.g,s.g),
                     blend_Darken(b.b,s.b)};
}

// lighten s
inline float blend_Lighten(float b, float s) {
    return fmax(s,b);
}

inline color_f_t blend_Lighten(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_Lighten(b.r,s.r),
                     blend_Lighten(b.g,s.g),
                     blend_Lighten(b.b,s.b)};
}

// color-dodge s
inline float blend_ColorDodge(float b, float s) {
    return (s==1.0)?s:fmin(b/(1.0-s),1.0);
}

inline color_f_t blend_ColorDodge(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_ColorDodge(b.r,s.r),
                     blend_ColorDodge(b.g,s.g),
                     blend_ColorDodge(b.b,s.b)};
}

// color-burn s
inline float blend_ColorBurn(float b, float s) {
    return (s==0.0)?s:fmax((1.0-((1.0-b)/s)),0.0);
}

inline color_f_t blend_ColorBurn(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_ColorBurn(b.r,s.r),
                     blend_ColorBurn(b.g,s.g),
                     blend_ColorBurn(b.b,s.b)};
}

// hard-light s
inline color_f_t blend_HardLight(const color_f_t &b, const color_f_t &s) {
    return blend_Overlay(s,b);
}

// soft-light s
inline float blend_SoftLight(float b, float s) {
    return (s<0.5) ? (2.0*b*s+b*b*(1.0-2.0*s)) : (sqrt(b)*(2.0*s-1.0)+2.0*b*(1.0-s));
}

inline color_f_t blend_SoftLight(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_SoftLight(b.r,s.r),
                     blend_SoftLight(b.g,s.g),
                     blend_SoftLight(b.b,s.b)};
}

// difference s
inline color_f_t blend_Difference(const color_f_t &b, const color_f_t &s) {
    return color_f_t{abs(b.r-s.r),
                     abs(b.g-s.g),
                     abs(b.b-s.b)};
}

// exclusion s
inline color_f_t blend_Exclusion(const color_f_t &b, const color_f_t &s) {
    color_f_t res;

    res.r = b.r + s.r - 2.0*b.r*s.r;
    res.g = b.g + s.g - 2.0*b.g*s.g;
    res.b = b.b + s.b - 2.0*b.b*s.b;

    return res;
}

// linear-burn

inline float blend_LinearBurn(float b, float s) {
    return fmax(b + s - 1.0, 0.0f);
}

inline color_f_t blend_LinearBurn(const color_f_t &b, const color_f_t &s) {
    color_f_t res;

    res.r = fmax(b.r + s.r - 1.0, 0.0f);
    res.g = fmax(b.g + s.g - 1.0, 0.0f);
    res.b = fmax(b.b + s.b - 1.0, 0.0f);

    return res;
}

// linear-dodge s
inline float blend_LinearDodge(float b, float s) {
    // Note : Same implementation as sAddf
    return fmin(b+s,1.0);
}

inline color_f_t blend_LinearDodge(const color_f_t &b, const color_f_t &s) {
    // Note : Same implementation as sAdd
    color_f_t res;

    res.r = fmin(b.r + s.r, 1.0f);
    res.g = fmin(b.g + s.g, 1.0f);
    res.b = fmin(b.b + s.b, 1.0f);

    return res;
}

// linear-light s
inline float blend_LinearLight(float b, float s) {
    return (s < 0.5) ? blend_LinearBurn(b,(2.0*s)) : blend_LinearBurn(b,(2.0*(s-0.5)));
}

inline color_f_t blend_LinearLight(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_LinearLight(b.r,s.r),
                     blend_LinearLight(b.g,s.g),
                     blend_LinearLight(b.b,s.b)};
}

// negation s
inline color_f_t blend_Negation(const color_f_t &b, const color_f_t &s) {
    color_f_t res;

    res.r = 1.0 - abs(1.0 - b.r - s.r);
    res.g = 1.0 - abs(1.0 - b.g - s.g);
    res.b = 1.0 - abs(1.0 - b.b - s.b);

    return res;
}

// pin-light s
inline float blend_PinLight(float b, float s) {
    return (s < 0.5) ? blend_Darken(b,(2.0*s)) : blend_Lighten(b,(2.0*(s-0.5)));
}

inline color_f_t blend_PinLight(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_PinLight(b.r,s.r),
                     blend_PinLight(b.g,s.g),
                     blend_PinLight(b.b,s.b)};
}

// reflect s
inline float blend_Reflect(float b, float s) {
    return (s==1.0)?s:fmin(b*b/(1.0-s),1.0);
}

inline color_f_t blend_Reflect(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_Reflect(b.r,s.r),
                     blend_Reflect(b.g,s.g),
                     blend_Reflect(b.b,s.b)};
}

// substract s
inline float blend_Substract(float b, float s) {
    return fmax(b+s-1.0,0.0);
}

inline color_f_t blend_Substract(const color_f_t &b, const color_f_t &s) {

    return color_f_t {blend_Substract(b.r, s.r),
                      blend_Substract(b.g, s.g),
                      blend_Substract(b.b, s.b)};
}

// substract s
inline float blend_Subtract(float b, float s) {
    return fmax(b-s, 0.0);
}

inline color_f_t blend_Subtract(const color_f_t &b, const color_f_t &s) {
    return color_f_t {blend_Subtract(b.r, s.r),
                      blend_Subtract(b.g, s.g),
                      blend_Subtract(b.b, s.b)};
}

// vivid-light s
inline float blend_VividLight(float b, float s) {
    return (s < 0.5) ? blend_ColorBurn(b, (2.0*s)) : blend_ColorDodge(b,(2.0*(s-0.5)));
}

inline color_f_t blend_VividLight(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_VividLight(b.r,s.r),
                     blend_VividLight(b.g,s.g),
                     blend_VividLight(b.b,s.b)};
}

// hard-mix s
inline float blend_HardMix(float b, float s) {
    return (blend_VividLight(b,s)<0.5)?0.0:1.0;
}

inline color_f_t blend_HardMix(const color_f_t &b, const color_f_t &s) {
    return color_f_t{blend_HardMix(b.r,s.r),
                     blend_HardMix(b.g,s.g),
                     blend_HardMix(b.b,s.b)};
}


// select and apply s mode by identifier.
// sing assumes that colors are NOT alpha pre-multiplied
inline color_f_t blend_mode_apply(BlendMode mode, const color_f_t &b, const color_f_t &s) {
    switch (mode) {

        case PassThrough:return s;
        case None:return s;
        case Normal:return s;
        case Dissolve:return s;
        case Darken:return blend_Darken(b, s);
        case Multiply:return blend_Multiply(b, s);
        case ColorBurn:return blend_ColorBurn(b, s);
        case LinearBurn:return blend_LinearBurn(b, s);
        case DarkerColor:return s;
        case Lighten:return blend_Lighten(b, s);
        case Screen:return blend_Screen(b, s);
        case ColorDodge:return blend_ColorDodge(b, s);
        case LinearDodge:return blend_LinearDodge(b, s);
        case LighterColor:return s;
        case Overlay:return blend_Overlay(b, s);
        case SoftLight:return blend_SoftLight(b, s);
        case HardLight: return blend_HardLight(b, s);
        case VividLight:return blend_VividLight(b, s);
        case LinearLight:return blend_LinearLight(b, s);
        case PinLight:return blend_PinLight(b, s);
        case HardMix:return blend_HardMix(b, s);
        case Difference:return blend_Difference(b, s);
        case Exclusion:return blend_Exclusion(b, s);
        case Subtract:return blend_Subtract(b, s);
        case Divide:return blend_Divide(b, s);
        case Hue:return s;
        case Saturation:return s;
        case Color:return s;
        case Luminosity:return s;
    }

    return s;
}

#pragma clang diagnostic pop