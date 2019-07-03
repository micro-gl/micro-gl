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

// a0 = αs x Fa + αb x Fb
// co = αs x Fa x Cs + αb x Fb x Cb
// according to PDF spec, page 322, if we use source-over
// result is NOT alpha pre-multiplied color
inline color_f_t porter_duff(float Fa, float Fb, const color_f_t & b, const color_f_t & s) {
    color_f_t result{};
    float as = s.a;
    float ab = b.a;

    float as_Fa = as * Fa;
    float ab_Fb = ab * Fb;

    result.a = as_Fa + ab_Fb;

    if(abs(result.a - 0.0) > 0.000001f) {
        // unmultiply alpha since the Porter-Duff equation results
        // in pre-multiplied alpha colors
        //result.rgb = (as * Fa * Cs + ab * Fb * Cb) / result.a;

        result.r = (as_Fa * s.r + ab_Fb * b.r);
        result.g = (as_Fa * s.g + ab_Fb * b.g);
        result.b = (as_Fa * s.b + ab_Fb * b.b);

        if(result.a != 1.0f) {
            result.r /= result.a;
            result.g /= result.a;
            result.b /= result.a;
        }
    }

    return result;
}

// porter-duff clear compositing equation
inline color_f_t porter_duff_Clear(const color_f_t & b, const color_f_t & s) {
    return porter_duff(0.0, 0.0, b, s);
}

// porter-duff copy compositing equation
inline color_f_t porter_duff_Copy(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0, 0.0, b, s);
}

// porter-duff Destination compositing equation
inline color_f_t porter_duff_Destination(const color_f_t & b, const color_f_t & s) {
    return porter_duff(0.0, 1.0, b, s);
}

// porter-duff Source compositing equation
inline color_f_t porter_duff_Source(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0, 0.0, b, s);
}

// porter-duff source-over compositing equation
inline color_f_t porter_duff_SourceOver(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0, 1.0 - float(s.a), b, s);
}

// porter-duff Destination Over compositing equation
inline color_f_t porter_duff_DestinationOver(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0 - float(b.a), 1.0, b, s);
}

// porter-duff Source In compositing equation
inline color_f_t porter_duff_SourceIn(const color_f_t & b, const color_f_t & s) {
    return porter_duff(float(b.a), 0.0, b, s);
}

// porter-duff Destination In compositing equation
inline color_f_t porter_duff_DestinationIn(const color_f_t & b, const color_f_t & s) {
    return porter_duff(0.0, float(s.a), b, s);
}

// porter-duff Source Out compositing equation
inline color_f_t porter_duff_SourceOut(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0 - float(b.a), 0.0, b, s);
}

// porter-duff Destination Out compositing equation
inline color_f_t porter_duff_DestinationOut(const color_f_t & b, const color_f_t & s) {
    return porter_duff(0.0, 1.0 - float(s.a), b, s);
}

// porter-duff Source Atop compositing equation
inline color_f_t porter_duff_SourceAtop(const color_f_t & b, const color_f_t & s) {
    return porter_duff(float(b.a), 1.0 - float(s.a), b, s);
}

// porter-duff Destination Atop compositing equation
inline color_f_t porter_duff_DestinationAtop(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0 - float(b.a), float(s.a), b, s);
}

// porter-duff XOR compositing equation
inline color_f_t porter_duff_XOR(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0 - float(b.a), 1.0 - float(s.a), b, s);
}

// porter-duff Destination Atop compositing equation
inline color_f_t porter_duff_Lighter(const color_f_t & b, const color_f_t & s) {
    return porter_duff(1.0, 1.0, b, s);
}

// select porter duff by mode identifier
inline color_f_t porter_duff_apply(PorterDuff mode, const color_f_t & b, const color_f_t & s) {
    switch (mode) {
        case SourceOver:return porter_duff_SourceOver(b, s);
        case Clear:return porter_duff_Clear(b, s);
        case Copy:return porter_duff_Copy(b, s);
        case Destination:return porter_duff_Destination(b, s);
        case DestinationOver:return porter_duff_DestinationOver(b, s);
        case SourceIn:return porter_duff_SourceIn(b, s);
        case DestinationIn:return porter_duff_DestinationIn(b, s);
        case SourceOut:return porter_duff_SourceOut(b, s);
        case DestinationOut:return porter_duff_DestinationOut(b, s);
        case SourceAtop:return porter_duff_SourceAtop(b, s);
        case DestinationAtop:return porter_duff_DestinationAtop(b, s);
        case XOR:return porter_duff_XOR(b, s);
        case Lighter:return porter_duff_Lighter(b, s);
        case Source:return porter_duff_Source(b, s);
    }

    return s;
}