#pragma once

#include <microgl/PorterDuff.h>

namespace porterduff {

    class None : public PorterDuffBase<None> {
    public:
        inline static const char * type() {
            return "None";
        }

        inline static void composite(const color_t & b,
                                     const color_t & s,
                                     color_t &output,
                                     const unsigned int alpha_bits,
                                     bool multiplied_alpha_result=false) {

            output.r=s.r;output.g=s.g;output.b=s.b;output.a=s.a;
            if(multiplied_alpha_result) {
                output.r = (output.r*output.a)>>alpha_bits;
                output.g = (output.g*output.a)>>alpha_bits;
                output.b = (output.b*output.a)>>alpha_bits;
            }
        }

        inline static void composite(const color_f_t & b,
                                     const color_f_t & s,
                                     color_f_t &output,
                                     bool multiplied_alpha_result=false) {
            output.r=s.r;output.g=s.g;output.b=s.b;output.a=s.a;
            if(multiplied_alpha_result) {
                output.r *= output.a;
                output.g *= output.a;
                output.b *= output.a;
            }
        }

    };

}
