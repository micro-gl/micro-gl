#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        channel convert_channel_fast_inaccurate(channel input, bits input_bits, bits output_bits) {
            if (output_bits == input_bits)
                return input;
            else if (input_bits > output_bits)
                return input >> (output_bits - input_bits);
            else
                return input << (input_bits - output_bits);
        }

        channel convert_channel_correct(channel input, bits input_bits, bits output_bits) {
            if (output_bits == input_bits)
                return input;
            unsigned int max_input= (1<<input_bits)-1;
            unsigned int max_output= (1<<output_bits)-1;
            return ((unsigned int)input*max_output)/max_input;
        }

        void convert_color(const color_t &input, color_t &output,
                           const bits &input_r_bits, const bits &input_g_bits,
                           const bits &input_b_bits, const bits &input_a_bits,
                           const bits &output_r_bits, const bits &output_g_bits,
                           const bits &output_b_bits, const bits &output_a_bits) {
            output.r = convert_channel_correct(input.r, input_r_bits, output_r_bits);
            output.g = convert_channel_correct(input.g, input_g_bits, output_g_bits);
            output.b = convert_channel_correct(input.b, input_b_bits, output_b_bits);
            output.a = convert_channel_correct(input.a, input_a_bits, output_a_bits);
        }

        void convert_color(const color_t &input, color_t &output) {
            convert_color(input, output, input.r_bits, input.g_bits, input.b_bits, input.a_bits,
                          output.r_bits, output.g_bits, output.b_bits, output.a_bits);
        }

    }
}