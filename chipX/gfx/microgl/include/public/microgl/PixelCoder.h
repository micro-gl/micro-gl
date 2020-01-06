#pragma once

#include <microgl/color.h>
#include "crpt.h"

namespace coder {
    using namespace microgl::color;
    using channel = unsigned char;
    using bits = unsigned char;

    channel convert_channel(channel input, bits input_bits, bits output_bits);
    void convert_color(const color_t &input, color_t &output,
                       const bits &input_r_bits, const bits &input_g_bits,
                       const bits &input_b_bits, const bits &input_a_bits,
                       const bits &output_r_bits, const bits &output_g_bits,
                       const bits &output_b_bits, const bits &output_a_bits);
    void convert_color(const color_t &input, color_t &output);
    void convert_color(const color_f_t &input, color_t &output,
                       const bits &output_r_bits, const bits &output_g_bits,
                       const bits &output_b_bits, const bits &output_a_bits);
    void convert_color(const color_t &input, color_f_t &output,
                       const bits &input_r_bits, const bits &input_g_bits,
                       const bits &input_b_bits, const bits &input_a_bits);
    void convert_color(const color_t &input, color_f_t &output);

    template<typename P, typename IMPL>
    class PixelCoder : public crpt<IMPL> {
    public:

        // integer colors
        static
        void encode(const color_t &input, P &output) {
            IMPL::encode(input, output);
        }
        static
        void decode(const P &input, color_t &output) {
            IMPL::decode(input, output);
            //update_channel_bit(output);
        }

        // float colors
        static
        void encode(const color_f_t &input, P &output) {
            color_t int_color{};
            convert_color(input, int_color, red_bits(), green_bits(), blue_bits(), alpha_bits());
            encode(int_color, output);
        }

        static
        void decode(const P &input, color_f_t &output) {
            color_t int_color{};
            decode(input, int_color);
            convert_color(int_color, output);
        }

        static
        channel red_bits() {
            return IMPL::red_bits();
        }

        static
        channel green_bits() {
            return IMPL::green_bits();
        }

        static
        channel blue_bits() {
            return IMPL::blue_bits();
        }

        static
        channel alpha_bits() {
            return IMPL::alpha_bits();
        }

        static
        void update_channel_bit(color_t &color) {
            color.r_bits = IMPL::red_bits();
            color.g_bits = IMPL::green_bits();
            color.b_bits = IMPL::blue_bits();
            color.a_bits = IMPL::alpha_bits();
        }

        static
        void convert(const color_f_t &input, color_t &output) {
            convert_color(input, output, red_bits(), green_bits(), blue_bits(), alpha_bits());
        }

        static
        void convert(const color_t &input, color_f_t &output) {
            convert_color(input, output, red_bits(), green_bits(), blue_bits(), alpha_bits());
        }

        template <typename P2, typename CODER2>
        static
        void convert(const color_t &input, color_t &output, const PixelCoder<P2, CODER2> & coder2) {
            // convert input color of my space into a color in coder2 space
            coder::convert_color(
                    input, output,
                    red_bits(), green_bits(), blue_bits(), alpha_bits(),
                    coder2.red_bits(), coder2.green_bits(), coder2.blue_bits(), coder2.alpha_bits());
        }

        inline const char * format() {
            return IMPL::format();
        };

    };

}
