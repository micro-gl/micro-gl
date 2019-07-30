#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemberFunctionCanBeStaticInspection"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma clang diagnostic ignored "-Wconstant-conversion"
#pragma ide diagnostic ignored "OCUnusedStructInspection"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma once

#include "Types.h"
#include "CRPT.h"
#include <math.h>
#include<algorithm>

namespace porterduff {
#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)

    enum class type {
        None,
        Clear,
        Copy,
        Destination,
        SourceOver,
        DestinationOver,
        SourceIn,
        DestinationIn,
        SourceOut,
        DestinationOut,
        SourceAtop,
        DestinationAtop,
        XOR,
        Lighter,
        Source,

        SourceOverOnOpaque,
    };

    template<typename IMPL>
    class PorterDuffBase : public CRPT<IMPL> {
    public:

        inline static void composite(const color_t & b, const color_t & s, color_t &output, const unsigned int alpha_bits) {
            IMPL::composite(b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b, const color_f_t & s, color_f_t &output) {
            IMPL::composite(b, s, output);
        }

    protected:
        inline static enum type type() {
            return IMPL::type();
        }

        // a0 = αs x Fa + αb x Fb
        // co = αs x Fa x Cs + αb x Fb x Cb
        // according to PDF spec, page 322, if we use source-over
        // result is NOT alpha pre-multiplied color
        inline static void internal_porter_duff(float Fa, float Fb,
                                                const color_f_t & b,
                                                const color_f_t & s,
                                                color_f_t &output) {
            float as = s.a;
            float ab = b.a;

            float as_Fa = as * Fa;
            float ab_Fb = ab * Fb;

            output.a = as_Fa + ab_Fb;

            if(fabs(output.a - 0.0) > 0.000001f) {
                // unmultiply alpha since the Porter-Duff equation results
                // in pre-multiplied alpha colors
                //result.rgb = (as * Fa * Cs + ab * Fb * Cb) / result.a;

                output.r = (as_Fa * s.r + ab_Fb * b.r);
                output.g = (as_Fa * s.g + ab_Fb * b.g);
                output.b = (as_Fa * s.b + ab_Fb * b.b);

                if(fabs(output.a - 1.0f) > 0.00001) {
                    output.r /= output.a;
                    output.g /= output.a;
                    output.b /= output.a;
                }
            }

        }

        // a0 = αs x Fa + αb x Fb
        // co = αs x Fa x Cs + αb x Fb x Cb
        // according to PDF spec, page 322, if we use source-over
        // result is NOT alpha pre-multiplied color
        inline static void internal_porter_duff(int Fa, int Fb,
                    const color_t & b,
                    const color_t & s,
                    color_t &output,
                    const unsigned int alpha_bits) {

            unsigned int as = s.a; // 128
            unsigned int ab = b.a;  // 255

            unsigned int max_alpha_range = (1<<alpha_bits);
            unsigned int max_alpha_value = max_alpha_range - 1;
            unsigned int double_alpha_max_range = (1<<(alpha_bits+alpha_bits));


            // these are less than 2^16
            unsigned int as_Fa = as * Fa; // 128 * 255
            unsigned int ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
            unsigned int combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits

            // cheaper alpha with bit shifting
            output.a = combined >> alpha_bits;

            // unmultiplied-alpha result

            // cheap optimization for opaque backdrops
            if(combined==double_alpha_max_range - (1<<(alpha_bits+1)) + 1) {
                const unsigned int alpha_bits_double = alpha_bits<<1;

                output.r = (as_Fa * s.r + ab_Fb * b.r) >> (alpha_bits_double); // inner expression is at most 26 bits, so no overflow
                output.g = (as_Fa * s.g + ab_Fb * b.g) >> (alpha_bits_double);
                output.b = (as_Fa * s.b + ab_Fb * b.b) >> (alpha_bits_double);
                return;
            }

            if(combined) {
                output.r = (as_Fa * s.r + ab_Fb * b.r)/ combined;
                output.g = (as_Fa * s.g + ab_Fb * b.g)/ combined;
                output.b = (as_Fa * s.b + ab_Fb * b.b)/ combined;
            }

            // multiplied-alpha result

//            if(combined) {
//                const unsigned int alpha_bits_double = alpha_bits + alpha_bits;
//                output.r = (as_Fa * s.r + ab_Fb * b.r) >> alpha_bits_double; // maybe ????
//                output.g = (as_Fa * s.g + ab_Fb * b.g) >> alpha_bits_double;
//                output.b = (as_Fa * s.b + ab_Fb * b.b) >> alpha_bits_double;
//            }

        }

    };
#define MAX_VAL_BITS2(bits) ((1<<(bits)) - 1)

    class None : public PorterDuffBase<None> {
    public:
        inline static enum type type() {
            return type::None;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {

            output.r=s.r;output.g=s.g;output.b=s.b;output.a=s.a;
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            output.r=s.r;output.g=s.g;output.b=s.b;output.a=s.a;
        }

    };

    class Clear : public PorterDuffBase<Clear> {
    public:
        inline static enum type type() {
            return type::Clear;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {

            internal_porter_duff(0, 0, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(0.0, 0.0, b, s, output);
        }

    };

    class Copy : public PorterDuffBase<Copy> {
    public:

        inline static enum type type() {
            return type::Copy;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val, 0, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0, 0.0, b, s, output);
        }

    };

    class Destination : public PorterDuffBase<Destination> {
    public:
        inline static enum type type() {
            return type::Destination;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(0, max_val, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(0.0, 1.0, b, s, output);
        }

    };

    class Source : public PorterDuffBase<Source> {
    public:
        inline static enum type type() {
            return type::Source;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val, 0, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0, 0.0, b, s, output);
        }

    };


    class SourceOver : public PorterDuffBase<SourceOver> {
    public:
        inline static enum type type() {
            return type::SourceOver;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            // opaque background optimization
//            if(b.a==max_val) {
//                unsigned int comp = max_val - s.a;
//                output.r = (s.a*s.r + comp * b.r) >> alpha_bits;
//                output.g = (s.a*s.g + comp * b.g) >> alpha_bits;
//                output.b = (s.a*s.b + comp * b.b) >> alpha_bits;
//                output.a = max_val;
//
//                return;
//            }

            internal_porter_duff(max_val, max_val - s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0, 1.0 - float(s.a), b, s, output);
        }

    };

    class SourceOverOnOpaque : public PorterDuffBase<SourceOverOnOpaque> {
    public:
        inline static enum type type() {
            return type::SourceOver;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            // this is to avoid branching as well
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);
//            unsigned int comp = max_val - s.a;
            unsigned int comp = s.a^max_val; // flipping bits equals (max_val - s.a)

            if(comp) {
                output.r = (s.a*s.r + comp * b.r) >> alpha_bits;
                output.g = (s.a*s.g + comp * b.g) >> alpha_bits;
                output.b = (s.a*s.b + comp * b.b) >> alpha_bits;
            } else {
                output.r = s.r;
                output.g = s.g;
                output.b = s.b;
            }

            output.a = max_val;
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0, 1.0 - float(s.a), b, s, output);
        }

    };

    class DestinationOver : public PorterDuffBase<DestinationOver> {
    public:
        inline static enum type type() {
            return type::DestinationOver;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val-b.a, max_val, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0 - float(b.a), 1.0, b, s, output);
        }

    };

    class SourceIn : public PorterDuffBase<SourceIn> {
    public:
        inline static enum type type() {
            return type::SourceIn;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(b.a, 0, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(float(b.a), 0.0, b, s, output);
        }

    };

    class DestinationIn : public PorterDuffBase<DestinationIn> {
    public:
        inline static enum type type() {
            return type::DestinationIn;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(0, s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(0.0, float(s.a), b, s, output);
        }

    };

    class SourceOut : public PorterDuffBase<SourceOut> {
    public:
        inline static enum type type() {
            return type::SourceOut;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val-b.a, 0, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0 - float(b.a), 0.0, b, s, output);
        }

    };

    class DestinationOut : public PorterDuffBase<DestinationOut> {
    public:
        inline static enum type type() {
            return type::DestinationOut;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(0, max_val-s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(0.0, 1.0 - float(s.a), b, s, output);
        }

    };

    class SourceAtop : public PorterDuffBase<SourceAtop> {
    public:
        inline static enum type type() {
            return type::SourceAtop;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(b.a, max_val-s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(float(b.a), 1.0 - float(s.a), b, s, output);
        }

    };

    class DestinationAtop : public PorterDuffBase<DestinationAtop> {
    public:
        inline static enum type type() {
            return type::DestinationAtop;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val-b.a, s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0 - float(b.a), float(s.a), b, s, output);
        }

    };

    class XOR : public PorterDuffBase<XOR> {
    public:
        inline static enum type type() {
            return type::XOR;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val-b.a, max_val-s.a, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0 - float(b.a), 1.0 - float(s.a), b, s, output);
        }

    };

    class Lighter : public PorterDuffBase<Lighter> {
    public:
        inline static enum type type() {
            return type::Lighter;
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits) {
            unsigned int max_val = MAX_VAL_BITS2(alpha_bits);

            internal_porter_duff(max_val, max_val, b, s, output, alpha_bits);
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output) {
            internal_porter_duff(1.0, 1.0, b, s, output);
        }

    };

}

#pragma clang diagnostic pop