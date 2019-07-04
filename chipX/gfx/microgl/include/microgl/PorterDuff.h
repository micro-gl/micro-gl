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

//
// float point versions
//
// a0 = αs x Fa + αb x Fb
// co = αs x Fa x Cs + αb x Fb x Cb
// according to PDF spec, page 322, if we use source-over
// result is NOT alpha pre-multiplied color
inline void porter_duff(float Fa, float Fb, const color_f_t & b, const color_f_t & s, color_f_t &output) {
    float as = s.a;
    float ab = b.a;

    float as_Fa = as * Fa;
    float ab_Fb = ab * Fb;

    output.a = as_Fa + ab_Fb;

    if(abs(output.a - 0.0) > 0.000001f) {
        // unmultiply alpha since the Porter-Duff equation results
        // in pre-multiplied alpha colors
        //result.rgb = (as * Fa * Cs + ab * Fb * Cb) / result.a;

        output.r = (as_Fa * s.r + ab_Fb * b.r);
        output.g = (as_Fa * s.g + ab_Fb * b.g);
        output.b = (as_Fa * s.b + ab_Fb * b.b);

        if(output.a != 1.0f) {
            output.r /= output.a;
            output.g /= output.a;
            output.b /= output.a;
        }
    }

}

// porter-duff clear compositing equation
inline void porter_duff_Clear(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(0.0, 0.0, b, s, output);
}

// porter-duff copy compositing equation
inline void porter_duff_Copy(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0, 0.0, b, s, output);
}

// porter-duff Destination compositing equation
inline void porter_duff_Destination(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(0.0, 1.0, b, s, output);
}

// porter-duff Source compositing equation
inline void porter_duff_Source(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0, 0.0, b, s, output);
}

// porter-duff source-over compositing equation
inline void porter_duff_SourceOver(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0, 1.0 - float(s.a), b, s, output);
}

// porter-duff Destination Over compositing equation
inline void porter_duff_DestinationOver(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0 - float(b.a), 1.0, b, s, output);
}

// porter-duff Source In compositing equation
inline void porter_duff_SourceIn(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(float(b.a), 0.0, b, s, output);
}

// porter-duff Destination In compositing equation
inline void porter_duff_DestinationIn(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(0.0, float(s.a), b, s, output);
}

// porter-duff Source Out compositing equation
inline void porter_duff_SourceOut(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0 - float(b.a), 0.0, b, s, output);
}

// porter-duff Destination Out compositing equation
inline void porter_duff_DestinationOut(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(0.0, 1.0 - float(s.a), b, s, output);
}

// porter-duff Source Atop compositing equation
inline void porter_duff_SourceAtop(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(float(b.a), 1.0 - float(s.a), b, s, output);
}

// porter-duff Destination Atop compositing equation
inline void porter_duff_DestinationAtop(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0 - float(b.a), float(s.a), b, s, output);
}

// porter-duff XOR compositing equation
inline void porter_duff_XOR(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0 - float(b.a), 1.0 - float(s.a), b, s, output);
}

// porter-duff Destination Atop compositing equation
inline void porter_duff_Lighter(const color_f_t & b, const color_f_t & s, color_f_t &output) {
    porter_duff(1.0, 1.0, b, s, output);
}

// select porter duff by mode identifier
inline void porter_duff_apply(PorterDuff mode, const color_f_t & b, const color_f_t & s, color_f_t &output) {
    switch (mode) {
        case SourceOver:porter_duff_SourceOver(b, s, output); break;
        case Clear:porter_duff_Clear(b, s, output); break;
        case Copy:porter_duff_Copy(b, s, output); break;
        case Destination:porter_duff_Destination(b, s, output); break;
        case DestinationOver:porter_duff_DestinationOver(b, s, output); break;
        case SourceIn:porter_duff_SourceIn(b, s, output); break;
        case DestinationIn:porter_duff_DestinationIn(b, s, output); break;
        case SourceOut:porter_duff_SourceOut(b, s, output); break;
        case DestinationOut:porter_duff_DestinationOut(b, s, output); break;
        case SourceAtop:porter_duff_SourceAtop(b, s, output); break;
        case DestinationAtop:porter_duff_DestinationAtop(b, s, output); break;
        case XOR:porter_duff_XOR(b, s, output); break;
        case Lighter:porter_duff_Lighter(b, s, output); break;
        case Source:porter_duff_Source(b, s, output); break;
    }
}