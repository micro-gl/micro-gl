#include <microgl/PorterDuff.h>

namespace microgl {
    namespace porterduff {
        using channel = unsigned int;
        using uint = unsigned int;
        using cuint = const unsigned int;

//        void apply_porter_duff(int Fa, int Fb,
//                                      const color_t &b,
//                                      const color_t &s,
//                                      color_t &output,
//                                      const unsigned int alpha_bits2) {
////#define DIV(a, b) use_fpu ? cuint(float(a)/float(b)) : ((a)/(b))
//
//            cuint as = s.a; // 128
//            cuint ab = b.a;  // 255
//            constexpr int alpha_bits=8;//
//            constexpr int alpha_bits_double = alpha_bits << 1;
//
//            // these are less than 16 bits
//            cuint as_Fa = as * Fa; // 128 * 255
//            cuint ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
//            cuint combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits
//            // cheaper alpha with bit shifting
//            output.a = combined >> alpha_bits;
//            // this is multiplied alpha channels
//            cuint r_channel = as_Fa * s.r + ab_Fb * b.r;
//            cuint g_channel = as_Fa * s.g + ab_Fb * b.g;
//            cuint b_channel = as_Fa * s.b + ab_Fb * b.b;
//            // if desired result should be un multiplied
//            // keep multiplied result, but of course transform to
//            // the correct bit space
//            output.r = r_channel >> alpha_bits_double;
//            output.g = g_channel >> alpha_bits_double;
//            output.b = b_channel >> alpha_bits_double;
///*
//            cuint as = s.a; // 128
//            cuint ab = b.a;  // 255
//            cuint max = (1<<alpha_bits)-1;
//            // these are less than 16 bits
//            cuint as_Fa = as * Fa; // 128 * 255
//            cuint ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
//            cuint combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits
//            if(fast)
//                output.a =combined>>alpha_bits;
//            else {
//                output.a= DIV(combined, max);
//            }
//
//            // this is multiplied alpha channels
//            cuint r_channel = as_Fa * s.r + ab_Fb * b.r;
//            cuint g_channel = as_Fa * s.g + ab_Fb * b.g;
//            cuint b_channel = as_Fa * s.b + ab_Fb * b.b;
//            // if desired result should be un multiplied
////            if (multiplied_alpha_result) {
////                if(fast) {
//                    cuint alpha_bits_double = alpha_bits << 1;
//                    output.r = r_channel >> alpha_bits_double;
//                    output.g = g_channel >> alpha_bits_double;
//                    output.b = b_channel >> alpha_bits_double;
//
////                } else {
////                    cuint max_double = max*max;
////                    output.r = DIV(r_channel, max_double);
////                    output.g = DIV(g_channel, max_double);
////                    output.b = DIV(b_channel, max_double);
////                }
////            } else {
////                if (combined) {
////                    output.r = DIV(r_channel, combined);
////                    output.g = DIV(g_channel, combined);
////                    output.b = DIV(b_channel, combined);
////                }
////            }
//*/
////#undef DIV
//        }

        // a0 = αs x Fa + αb x Fb
        // co = αs x Fa x Cs + αb x Fb x Cb
        // according to PDF spec, page 322, if we use source-over
        // result is NOT alpha pre-multiplied color
        void apply_porter_duff_accurate(int Fa, int Fb,
                                        const color_t &b,
                                        const color_t &s,
                                        color_t &output,
                                        const unsigned int alpha_bits,
                                        bool multiplied_alpha_result) {

            cuint as = s.a; // 128
            cuint ab = b.a;  // 255
            cuint max = (1<<alpha_bits)-1;
            cuint max_double = max*max;
            // these are less than 16 bits
            cuint as_Fa = as * Fa; // 128 * 255
            cuint ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
            cuint combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits
            output.a = combined/max;
            // this is multiplied alpha channels
            cuint r_channel = as_Fa * s.r + ab_Fb * b.r;
            cuint g_channel = as_Fa * s.g + ab_Fb * b.g;
            cuint b_channel = as_Fa * s.b + ab_Fb * b.b;
            // if desired result should be un multiplied
            if (!multiplied_alpha_result) {
//                if (combined) {
                    output.r = float(r_channel) / combined;
                    output.g = float(g_channel) / combined;
                    output.b = float(b_channel) / combined;
//                }
            } else {
                output.r = r_channel/max_double;
                output.g = g_channel/max_double;
                output.b = b_channel/max_double;
            }

        }

        // a0 = αs x Fa + αb x Fb
        // co = αs x Fa x Cs + αb x Fb x Cb
        // according to PDF spec, page 322, if we use source-over
        // result is NOT alpha pre-multiplied color
        void apply_porter_duff_fast(int Fa, int Fb,
                                    const color_t &b,
                                    const color_t &s,
                                    color_t &output,
                                    const unsigned int alpha_bits,
                                    bool multiplied_alpha_result) {

            cuint as = s.a; // 128
            cuint ab = b.a;  // 255
            cuint alpha_bits_double = alpha_bits << 1;

            // these are less than 16 bits
            cuint as_Fa = as * Fa; // 128 * 255
            cuint ab_Fb = ab * Fb; // 255 * (255 - 128) = 255 * 127
            cuint combined = as_Fa + ab_Fb; // 128 * 255 + 255 * 127 // may be 17 bits
            // cheaper alpha with bit shifting
            output.a = combined >> alpha_bits;
            // this is multiplied alpha channels
            cuint r_channel = as_Fa * s.r + ab_Fb * b.r;
            cuint g_channel = as_Fa * s.g + ab_Fb * b.g;
            cuint b_channel = as_Fa * s.b + ab_Fb * b.b;
            // if desired result should be un multiplied
            if (!multiplied_alpha_result) {
                if (combined) {
                    output.r = r_channel / combined;
                    output.g = g_channel / combined;
                    output.b = b_channel / combined;
                }
            } else {
                // keep multiplied result, but of course transform to
                // the correct bit space
                output.r = r_channel >> alpha_bits_double;
                output.g = g_channel >> alpha_bits_double;
                output.b = b_channel >> alpha_bits_double;
            }

        }

    }
}