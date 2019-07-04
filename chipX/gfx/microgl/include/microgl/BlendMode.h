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

// float point version

// overlay s
inline float blend_Overlay(float b, float s) {
    return b<0.5?(2.0*b*s):(1.0-2.0*(1.0-b)*(1.0-s));
}

inline void blend_Overlay(const color_f_t &b, const color_f_t &s, color_f_t & output) {

    output.r=blend_Overlay(b.r,s.r);
    output.g=blend_Overlay(b.g,s.g);
    output.b=blend_Overlay(b.b,s.b);
}

// normal s
inline color_f_t blend_Normal(const color_f_t &b, const color_f_t &s) {
    return s;
}


// multiply s
inline void blend_Multiply(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=b.r*s.r;
    output.g=b.g*s.g;
    output.b=b.b*s.b;
}

// divide s
inline float blend_Divide(float b, float s) {
    return (s==0.0) ? 1.0 : (b/s);
}

inline void blend_Divide(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_Divide(b.r,s.r);
    output.g=blend_Divide(b.g,s.g);
    output.b=blend_Divide(b.b,s.b);
}

// screen s
inline float blend_Screen(float b, float s) {
    return 1.0-((1.0-b)*(1.0-s));
}

inline void blend_Screen(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_Screen(b.r,s.r);
    output.g=blend_Screen(b.g,s.g);
    output.b=blend_Screen(b.b,s.b);
}

// darken s
inline float blend_Darken(float b, float s) {
    return fmin(s,b);
}

inline void blend_Darken(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_Darken(b.r,s.r);
    output.g=blend_Darken(b.g,s.g);
    output.b=blend_Darken(b.b,s.b);
}

// lighten s
inline float blend_Lighten(float b, float s) {
    return fmax(s,b);
}

inline void blend_Lighten(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_Lighten(b.r,s.r);
    output.g=blend_Lighten(b.g,s.g);
    output.b=blend_Lighten(b.b,s.b);
}

// color-dodge s
inline float blend_ColorDodge(float b, float s) {
    return (s==1.0)?s:fmin(b/(1.0-s),1.0);
}

inline void blend_ColorDodge(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_ColorDodge(b.r,s.r);
    output.g=blend_ColorDodge(b.g,s.g);
    output.b=blend_ColorDodge(b.b,s.b);
}

// color-burn s
inline float blend_ColorBurn(float b, float s) {
    return (s==0.0)?s:fmax((1.0-((1.0-b)/s)),0.0);
}

inline void blend_ColorBurn(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_ColorBurn(b.r,s.r);
    output.g=blend_ColorBurn(b.g,s.g);
    output.b=blend_ColorBurn(b.b,s.b);
}

// hard-light s
inline void blend_HardLight(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    blend_Overlay(s,b, output);
}

// soft-light s
inline float blend_SoftLight(float b, float s) {
    return (s<0.5) ? (2.0*b*s+b*b*(1.0-2.0*s)) : (sqrt(b)*(2.0*s-1.0)+2.0*b*(1.0-s));
}

inline void blend_SoftLight(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_SoftLight(b.r,s.r);
    output.g=blend_SoftLight(b.g,s.g);
    output.b=blend_SoftLight(b.b,s.b);
}

// difference s
inline void blend_Difference(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=abs(b.r-s.r);
    output.g=abs(b.g-s.g);
    output.b=abs(b.b-s.b);
}

// exclusion s
inline void blend_Exclusion(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r = b.r + s.r - 2.0*b.r*s.r;
    output.g = b.g + s.g - 2.0*b.g*s.g;
    output.b = b.b + s.b - 2.0*b.b*s.b;
}

// linear-burn

inline float blend_LinearBurn(float b, float s) {
    return fmax(b + s - 1.0, 0.0f);
}

inline void blend_LinearBurn(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r = fmax(b.r + s.r - 1.0, 0.0f);
    output.g = fmax(b.g + s.g - 1.0, 0.0f);
    output.b = fmax(b.b + s.b - 1.0, 0.0f);
}

// linear-dodge s
inline float blend_LinearDodge(float b, float s) {
    // Note : Same implementation as sAddf
    return fmin(b+s,1.0);
}

inline void blend_LinearDodge(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r = fmin(b.r + s.r, 1.0f);
    output.g = fmin(b.g + s.g, 1.0f);
    output.b = fmin(b.b + s.b, 1.0f);
}

// linear-light s
inline float blend_LinearLight(float b, float s) {
    return (s < 0.5) ? blend_LinearBurn(b,(2.0*s)) : blend_LinearBurn(b,(2.0*(s-0.5)));
}

inline void blend_LinearLight(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output = color_f_t{blend_LinearLight(b.r,s.r),
                     blend_LinearLight(b.g,s.g),
                     blend_LinearLight(b.b,s.b)};
}

// negation s
inline void blend_Negation(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r = 1.0 - abs(1.0 - b.r - s.r);
    output.g = 1.0 - abs(1.0 - b.g - s.g);
    output.b = 1.0 - abs(1.0 - b.b - s.b);
}

// pin-light s
inline float blend_PinLight(float b, float s) {
    return (s < 0.5) ? blend_Darken(b,(2.0*s)) : blend_Lighten(b,(2.0*(s-0.5)));
}

inline void blend_PinLight(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output = color_f_t{blend_PinLight(b.r,s.r),
                     blend_PinLight(b.g,s.g),
                     blend_PinLight(b.b,s.b)};
}

// reflect s
inline float blend_Reflect(float b, float s) {
    return (s==1.0)?s:fmin(b*b/(1.0-s),1.0);
}

inline void blend_Reflect(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_Reflect(b.r,s.r);
    output.g=blend_Reflect(b.g,s.g);
    output.b=blend_Reflect(b.b,s.b);
}

// substract s
inline float blend_Substract(float b, float s) {
    return fmax(b+s-1.0,0.0);
}

inline void blend_Substract(const color_f_t &b, const color_f_t &s, color_f_t & output) {

    output.r=blend_Substract(b.r, s.r);
    output.g=blend_Substract(b.g, s.g);
    output.b=blend_Substract(b.b, s.b);
}

// substract s
inline float blend_Subtract(float b, float s) {
    return fmax(b-s, 0.0);
}

inline void blend_Subtract(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_Subtract(b.r, s.r);
    output.g=blend_Subtract(b.g, s.g);
    output.b=blend_Subtract(b.b, s.b);
}

// vivid-light s
inline float blend_VividLight(float b, float s) {
    return (s < 0.5) ? blend_ColorBurn(b, (2.0*s)) : blend_ColorDodge(b,(2.0*(s-0.5)));
}

inline void blend_VividLight(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_VividLight(b.r,s.r);
    output.g=blend_VividLight(b.g,s.g);
    output.b=blend_VividLight(b.b,s.b);
}

// hard-mix s
inline float blend_HardMix(float b, float s) {
    return (blend_VividLight(b,s)<0.5)?0.0:1.0;
}

inline void blend_HardMix(const color_f_t &b, const color_f_t &s, color_f_t & output) {
    output.r=blend_HardMix(b.r,s.r);
    output.g=blend_HardMix(b.g,s.g);
    output.b=blend_HardMix(b.b,s.b);
}


// select and apply s mode by identifier.
// sing assumes that colors are NOT alpha pre-multiplied
inline void blend_mode_apply(BlendMode mode, const color_f_t &b, const color_f_t &s, color_f_t & output) {
    switch (mode) {
//        case PassThrough:return s;
//        case None:return s;
        case Multiply: blend_Multiply(b, s, output);break;
        case Normal:output=s;break;
        case Dissolve:output=s;break;
        case Darken: blend_Darken(b, s, output);break;
        case ColorBurn: blend_ColorBurn(b, s, output);break;
        case LinearBurn: blend_LinearBurn(b, s, output);break;
        case DarkerColor: output=s;break;
        case Lighten: blend_Lighten(b, s, output);break;
        case Screen: blend_Screen(b, s, output);break;
        case ColorDodge: blend_ColorDodge(b, s, output);break;
        case LinearDodge: blend_LinearDodge(b, s, output);break;
        case LighterColor: output=s;break;
        case Overlay: blend_Overlay(b, s, output);break;
        case SoftLight: blend_SoftLight(b, s, output);break;
        case HardLight:  blend_HardLight(b, s, output);break;
        case VividLight: blend_VividLight(b, s, output);break;
        case LinearLight: blend_LinearLight(b, s, output);break;
        case PinLight: blend_PinLight(b, s, output);break;
        case HardMix: blend_HardMix(b, s, output);break;
        case Difference: blend_Difference(b, s, output);break;
        case Exclusion: blend_Exclusion(b, s, output);break;
        case Subtract: blend_Subtract(b, s, output);break;
        case Divide: blend_Divide(b, s, output);break;
        default: output=s;break;
    }

}

#pragma clang diagnostic pop