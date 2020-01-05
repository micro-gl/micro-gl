#include <microgl/PixelCoder.h>

namespace coder {

    channel convert_channel(channel input, bits input_bits, bits output_bits) {
        if (output_bits == input_bits)
            return input;
        else if (input_bits > output_bits)
            return input >> (output_bits - input_bits);
        else
            return input << (input_bits - output_bits);
    }

    void convert_color(const color_t &input, color_t &output) {
        output.r = convert_channel(input.r, input.r_bits, output.r_bits);
        output.g = convert_channel(input.g, input.g_bits, output.g_bits);
        output.b = convert_channel(input.b, input.b_bits, output.b_bits);
        output.a = convert_channel(input.a, input.a_bits, output.a_bits);
    }

    void convert_color(const color_f_t &input, color_t &output,
                 const bits &r_bits, const bits &g_bits,
                 const bits &b_bits, const bits &a_bits) {
        output.r = channel(input.r*float((1u<<r_bits)-1));
        output.g = channel(input.g*float((1u<<g_bits)-1));
        output.b = channel(input.b*float((1u<<b_bits)-1));
        output.a = channel(input.a*float((1u<<a_bits)-1));
        output.r_bits = r_bits;
        output.g_bits = g_bits;
        output.b_bits = b_bits;
        output.a_bits = a_bits;
    }

    void convert_color(const color_t &input, color_f_t &output) {
        output.r = float(input.r)/float((1u<<input.r_bits)-1);
        output.g = float(input.g)/float((1u<<input.g_bits)-1);
        output.b = float(input.b)/float((1u<<input.b_bits)-1);
        output.a = float(input.a)/float((1u<<input.a_bits)-1);
    }

//    void convert(const color_t &input, color_f_t &output) {
//
//    }

}