#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemberFunctionCanBeStaticInspection"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma clang diagnostic ignored "-Wconstant-conversion"
#pragma ide diagnostic ignored "OCUnusedStructInspection"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma once

#include <microgl/vec2.h>
#include <microgl/color.h>
#include "CRPT.h"
#include <cmath>
#include<algorithm>
using namespace microgl;

namespace blendmode {
#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)

    enum class type {
        PassThrough,
        None,

        Normal,
        Dissolve,

        Darken,
        Multiply,
        ColorBurn,
        LinearBurn,
        DarkerColor,

        Lighten,
        Screen,
        ColorDodge,
        LinearDodge,
        LighterColor,

        Overlay,
        SoftLight,
        HardLight,
        VividLight,
        LinearLight,
        PinLight,
        HardMix,

        Difference,
        Exclusion,
        Subtract,
        Divide,

        Hue,
        Saturation,
        Color,
        Luminosity,
    };


    template<typename IMPL>
    class BlendModeBase : public CRPT<IMPL> {
    public:

        static inline void blend(const color_t &b,
                          const color_t &s,
                          color_t & output,
                          const uint8_t r_bits,
                          const uint8_t g_bits,
                          const uint8_t b_bits) {

            IMPL::blend(b, s, output, r_bits, g_bits, b_bits);
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            IMPL::blend(b, s, output);
        }

        static inline enum type type() {
            return IMPL::type();
        }

    };

//    template<typename IMPL>
//    class IntegerBlendModeBase : public BlendModeBase<color_t, IMPL> {
//    public:
//
//        static inline void blend(const color_t &b,
//                          const color_t &s,
//                          color_t & output,
//                          uint8_t r_bits,
//                          uint8_t g_bits,
//                          uint8_t b_bits) {
//
//            return IMPL::blend(b, s, output, r_bits, g_bits, b_bits);
//        }
//
//    };
//
//    template<typename IMPL>
//    class BlendModeBase : BlendModeBase<color_f_t, IMPL> {
//    public:
//
//        static inline void blend(const color_f_t &b,
//                          const color_f_t &s,
//                          color_f_t & output) {
//
//            return IMPL::blend(b, s, output);
//        }
//
//    };

    // float blend modes


    class Normal : public BlendModeBase<Normal> {
    public:

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r = s.r;
            output.g = s.g;
            output.b = s.b;

        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r = s.r;
            output.g = s.g;
            output.b = s.b;

        }

        static inline enum type type() {
            return type::Normal;
        }

    };


    class Multiply : public BlendModeBase<Multiply> {
    public:

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r = (b.r * s.r)>>r_bits;
            output.g = (b.g * s.g)>>g_bits;
            output.b = (b.b * s.b)>>b_bits;

        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r = (b.r * s.r);
            output.g = (b.g * s.g);
            output.b = (b.b * s.b);
        }

        static inline enum type type() {
            return type::Multiply;
        }

    };

    class Darken : public BlendModeBase<Darken> {
    public:

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=std::min(b.r,s.r);
            output.g=std::min(b.g,s.g);
            output.b=std::min(b.b,s.b);
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r=fmin(b.r,s.r);
            output.g=fmin(b.g,s.g);
            output.b=fmin(b.b,s.b);
        }

        static inline enum type type() {
            return type::Darken;
        }
    };

    class ColorBurn : public BlendModeBase<ColorBurn> {
    public:
        static inline int blend_ColorBurn(int b, int s, uint8_t bits) {
            int max = (1<<bits) - 1;

            return (s==0) ? s : std::max((max - ((max-b)<<bits)/s), 0);

//    return (s==0.0)?s:fmax((1.0-((1.0-b)/s)),0.0);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_ColorBurn(b.r, s.r, r_bits);
            output.g=blend_ColorBurn(b.g, s.g, g_bits);
            output.b=blend_ColorBurn(b.b, s.b, b_bits);
        }

        static inline float blend_ColorBurn(float b, float s) {
            return (s==0.0)?s:fmax((1.0-((1.0-b)/s)),0.0);
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r=blend_ColorBurn(b.r, s.r);
            output.g=blend_ColorBurn(b.g, s.g);
            output.b=blend_ColorBurn(b.b, s.b);
        }

        static inline enum type type() {
            return type::ColorBurn;
        }
    };

    class LinearBurn : public BlendModeBase<LinearBurn> {
    public:

        static inline int blend_LinearBurn(int b, int s, uint8_t bits) {
            return std::max(b + s - (1<<bits) - 1, 0);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r = blend_LinearBurn(b.r, s.r, r_bits);
            output.g = blend_LinearBurn(b.g, s.g, g_bits);
            output.b = blend_LinearBurn(b.b, s.b, b_bits);
        }

        static inline float blend_LinearBurn(float b, float s) {
            return fmax(b + s - 1.0, 0.0f);
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r = blend_LinearBurn(b.r, s.r);
            output.g = blend_LinearBurn(b.g, s.g);
            output.b = blend_LinearBurn(b.b, s.b);
        }

        static inline enum type type() {
            return type::LinearBurn;
        }
    };

    class Lighten : public BlendModeBase<Lighten> {
    public:

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r=fmax(b.r,s.r);
            output.g=fmax(b.g,s.g);
            output.b=fmax(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=std::max(b.r,s.r);
            output.g=std::max(b.g,s.g);
            output.b=std::max(b.b,s.b);
        }

        static inline enum type type() {
            return type::Lighten;
        }
    };

    class Screen : public BlendModeBase<Screen> {
    public:
        static inline int blend_Screen(int b, int s, int bits) {
            int max = (1<<bits) - 1;

            return max-(((max-b)*(max-s))>>bits);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_Screen(b.r,s.r,r_bits);
            output.g=blend_Screen(b.g,s.g,g_bits);
            output.b=blend_Screen(b.b,s.b,b_bits);
        }

        static inline float blend_Screen(float b, float s) {
            return 1.0-((1.0-b)*(1.0-s));
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r=blend_Screen(b.r,s.r);
            output.g=blend_Screen(b.g,s.g);
            output.b=blend_Screen(b.b,s.b);
        }

        static inline enum type type() {
            return type::Screen;
        }
    };

    class ColorDodge : public BlendModeBase<ColorDodge> {
    public:

        static inline int blend_ColorDodge(int b, int s, int bits) {
            int max = (1<<bits) - 1;

            return (s==max) ? s : std::min((b<<bits)/(max-s), max);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_ColorDodge(b.r, s.r, r_bits);
            output.g=blend_ColorDodge(b.g, s.g, g_bits);
            output.b=blend_ColorDodge(b.b, s.b, b_bits);
        }

        static inline float blend_ColorDodge(float b, float s) {
            return (s==1.0)?s:fmin(b/(1.0-s),1.0);
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r=blend_ColorDodge(b.r, s.r);
            output.g=blend_ColorDodge(b.g, s.g);
            output.b=blend_ColorDodge(b.b, s.b);
        }

        static inline enum type type() {
            return type::ColorDodge;
        }
    };

    class LinearDodge : public BlendModeBase<LinearDodge> {
    public:

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r = std::min(b.r + s.r, (1<<r_bits) - 1);
            output.g = std::min(b.g + s.g, (1<<g_bits) - 1);
            output.b = std::min(b.b + s.b, (1<<b_bits) - 1);
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r = fmin(b.r + s.r, 1.0f);
            output.g = fmin(b.g + s.g, 1.0f);
            output.b = fmin(b.b + s.b, 1.0f);
        }

        static inline enum type type() {
            return type::LinearDodge;
        }
    };

    class Overlay : public BlendModeBase<Overlay> {
    public:

        static inline int blend_Overlay(int b, int s, int bits) {
            int max = MAX_VAL_BITS(bits);

            return 2*b<max ? ((2*b*s)>>bits) : (max - ((2*(max-b)*(max-s))>>bits));
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=blend_Overlay(b.r,s.r, r_bits);
            output.g=blend_Overlay(b.g,s.g, g_bits);
            output.b=blend_Overlay(b.b,s.b, b_bits);
        }

        static inline float blend_Overlay(float b, float s) {
            return b<0.5?(2.0*b*s):(1.0-2.0*(1.0-b)*(1.0-s));
        }

        static inline void blend(const color_f_t &b,
                          const color_f_t &s,
                          color_f_t & output) {

            output.r=blend_Overlay(b.r, s.r);
            output.g=blend_Overlay(b.g, s.g);
            output.b=blend_Overlay(b.b ,s.b);
        }

        static inline enum type type() {
            return type::Overlay;
        }
    };

    class Subtract : public BlendModeBase<Subtract> {
    public:

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=fmax(b.r-s.r, 0);
            output.g=fmax(b.g-s.g, 0);
            output.b=fmax(b.b-s.b, 0);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

            output.r=std::max(b.r-s.r, 0);
            output.g=std::max(b.g-s.g, 0);
            output.b=std::max(b.b-s.b, 0);
        }

        static inline enum type type() {
            return type::Subtract;
        }
    };

    class SoftLight : public BlendModeBase<SoftLight> {
    public:

        static inline float blend_SoftLight(float b, float s) {
            return (s<0.5) ? (2.0*b*s+b*b*(1.0-2.0*s)) : (sqrt(b)*(2.0*s-1.0)+2.0*b*(1.0-s));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_SoftLight(b.r,s.r);
            output.g=blend_SoftLight(b.g,s.g);
            output.b=blend_SoftLight(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {

        }

        static inline enum type type() {
            return type::SoftLight;
        }
    };

    class HardLight : public BlendModeBase<HardLight> {
    public:

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            Overlay::blend(s, b, output);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            Overlay::blend(s, b, output, r_bits, g_bits, b_bits);

        }

        static inline enum type type() {
            return type::HardLight;
        }
    };

    class VividLight : public BlendModeBase<VividLight> {
    public:

        static inline float blend_ColorDodge(float b, float s) {
            return (s==1.0)?s:fmin(b/(1.0-s),1.0);
        }

        static inline float blend_VividLight(float b, float s) {
            return (s < 0.5) ? ColorBurn::blend_ColorBurn(b, (2.0*s)) : blend_ColorDodge(b,(2.0*(s-0.5)));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_VividLight(b.r,s.r);
            output.g=blend_VividLight(b.g,s.g);
            output.b=blend_VividLight(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline enum type type() {
            return type::VividLight;
        }
    };

    class LinearLight : public BlendModeBase<LinearLight> {
    public:

        static inline float blend_LinearLight(float b, float s) {
            return (s < 0.5) ? LinearBurn::blend_LinearBurn(b,(2.0*s)) : LinearBurn::blend_LinearBurn(b,(2.0*(s-0.5)));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_LinearLight(b.r,s.r);
            output.g=blend_LinearLight(b.g,s.g);
            output.b=blend_LinearLight(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline enum type type() {
            return type::LinearLight;
        }
    };

    class PinLight : public BlendModeBase<PinLight> {
    public:

        static inline float blend_Darken(float b, float s) {
            return fmin(s,b);
        }

        static inline float blend_Lighten(float b, float s) {
            return fmax(s,b);
        }

        static inline float blend_PinLight(float b, float s) {
            return (s < 0.5) ? blend_Darken(b,(2.0*s)) : blend_Lighten(b,(2.0*(s-0.5)));
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_PinLight(b.r,s.r);
            output.g=blend_PinLight(b.g,s.g);
            output.b=blend_PinLight(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline enum type type() {
            return type::PinLight;
        }
    };

    class HardMix : public BlendModeBase<HardMix> {
    public:

        static inline float blend_HardMix(float b, float s) {
            return (VividLight::blend_VividLight(b,s)<0.5)?0.0:1.0;
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_HardMix(b.r,s.r);
            output.g=blend_HardMix(b.g,s.g);
            output.b=blend_HardMix(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline enum type type() {
            return type::HardMix;
        }
    };

    class Difference : public BlendModeBase<Difference> {
    public:

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=fabs(b.r-s.r);
            output.g=fabs(b.g-s.g);
            output.b=fabs(b.b-s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            output.r=abs(b.r-s.r);
            output.g=abs(b.g-s.g);
            output.b=abs(b.b-s.b);
        }

        static inline enum type type() {
            return type::Difference;
        }
    };

    class Exclusion : public BlendModeBase<Exclusion> {
    public:

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r = b.r + s.r - 2.0*b.r*s.r;
            output.g = b.g + s.g - 2.0*b.g*s.g;
            output.b = b.b + s.b - 2.0*b.b*s.b;
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline enum type type() {
            return type::Exclusion;
        }
    };

    class Divide : public BlendModeBase<Divide> {
    public:
        static inline float blend_Divide(float b, float s) {
            return (s==0.0) ? 1.0 : (b/s);
        }

        static inline void blend(const color_f_t &b,
                                 const color_f_t &s,
                                 color_f_t & output) {

            output.r=blend_Divide(b.r,s.r);
            output.g=blend_Divide(b.g,s.g);
            output.b=blend_Divide(b.b,s.b);
        }

        static inline void blend(const color_t &b,
                                 const color_t &s,
                                 color_t & output,
                                 const uint8_t r_bits,
                                 const uint8_t g_bits,
                                 const uint8_t b_bits) {
            // todo
        }

        static inline enum type type() {
            return type::Divide;
        }
    };


}

#pragma clang diagnostic pop
