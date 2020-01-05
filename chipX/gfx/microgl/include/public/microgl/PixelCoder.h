#pragma once

#include <microgl/color.h>
#include "crpt.h"

namespace coder {
    using namespace microgl::color;
    using channel = unsigned char;
    using bits = unsigned char;

    channel convert_channel(channel input, bits input_bits, bits output_bits);
    void convert_color(const color_t &input, color_t &output);
    void convert_color(const color_f_t &input, color_t &output,
                 const bits &r_bits, const bits &g_bits,
                 const bits &b_bits, const bits &a_bits);
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
            P pixel;
            encode(input, pixel);
            decode(pixel, output);
        }

        static
        void convert(const color_t &input, color_f_t &output) {
            P pixel;
            encode(input, pixel);
            decode(pixel, output);
        }

        inline const char * format() {
            return IMPL::format();
        };

    };

}
