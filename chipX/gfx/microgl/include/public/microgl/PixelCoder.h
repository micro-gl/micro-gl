#pragma once

#include <microgl/color.h>
#include <microgl/intensity.h>
#include "crpt.h"

namespace microgl {
    namespace coder {
        using namespace microgl::color;
        using channel = unsigned char;
        using bits = unsigned char;

        channel convert_channel_fast_inaccurate(channel input, bits input_bits, bits output_bits);
        channel convert_channel_correct(channel input, bits input_bits, bits output_bits);

        void convert_color(const color_t &input, color_t &output,
                           const bits &input_r_bits, const bits &input_g_bits,
                           const bits &input_b_bits, const bits &input_a_bits,
                           const bits &output_r_bits, const bits &output_g_bits,
                           const bits &output_b_bits, const bits &output_a_bits);

        void convert_color(const color_t &input, color_t &output);

        template <typename number>
        void convert_color(const intensity<number> &input, color_t &output,
                           const bits &output_r_bits, const bits &output_g_bits,
                           const bits &output_b_bits, const bits &output_a_bits) {
            output.r = channel(input.r * number((1u << output_r_bits) - 1));
            output.g = channel(input.g * number((1u << output_g_bits) - 1));
            output.b = channel(input.b * number((1u << output_b_bits) - 1));
            output.a = channel(input.a * number((1u << output_a_bits) - 1));
            output.r_bits = output_r_bits;
            output.g_bits = output_g_bits;
            output.b_bits = output_b_bits;
            output.a_bits = output_a_bits;
        }

        template <typename number>
        void convert_color(const color_t &input, intensity<number> &output) {
            convert_color<number>(input, output, input.r_bits, input.g_bits,
                                  input.b_bits, input.a_bits);
        }

        template <typename number>
        void convert_color(const color_t &input, intensity<number> &output,
                           const bits &input_r_bits, const bits &input_g_bits,
                           const bits &input_b_bits, const bits &input_a_bits) {
            output.r = number(input.r)/number((1u << input_r_bits) - 1);
            output.g = number(input.g)/number((1u << input_g_bits) - 1);
            output.b = number(input.b)/number((1u << input_b_bits) - 1);
            output.a = number(input.a)/number((1u << input_a_bits) - 1);
        }

        template<typename P, typename IMPL>
        class PixelCoder : public crpt<IMPL> {
        public:
//            typedef P Pixel;
            using Pixel=P;
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

            template <typename number>
            static
            void encode(const intensity<number> &input, P &output) {
                color_t int_color{};
                convert_color(input, int_color, red_bits(), green_bits(), blue_bits(), alpha_bits());
                encode(int_color, output);
            }

            template <typename number>
            static
            void decode(const P &input, intensity<number> &output) {
                color_t int_color{};
                decode(input, int_color);
                convert_color(int_color, output, red_bits(), green_bits(), blue_bits(), alpha_bits());
            }

            static constexpr channel red_bits() { return IMPL::red_bits(); }
            static constexpr channel green_bits() { return IMPL::green_bits(); }
            static constexpr channel blue_bits() { return IMPL::blue_bits(); }
            static constexpr channel alpha_bits() { return IMPL::alpha_bits(); }

            static
            void update_channel_bit(color_t &color) {
                color.r_bits = IMPL::red_bits();
                color.g_bits = IMPL::green_bits();
                color.b_bits = IMPL::blue_bits();
                color.a_bits = IMPL::alpha_bits();
            }

            template <typename number>
            static
            void convert(const intensity<number> &input, color_t &output) {
                convert_color(input, output, red_bits(), green_bits(), blue_bits(), alpha_bits());
            }

            template <typename number>
            static
            void convert(const color_t &input, intensity<number> &output) {
                convert_color(input, output, red_bits(), green_bits(), blue_bits(), alpha_bits());
            }

            template<typename P2, typename CODER2>
            static
            void convert(const color_t &input, color_t &output, const PixelCoder<P2, CODER2> &coder2) {
                // convert input color of my space into a color in coder2 space
                coder::convert_color(
                        input, output,
                        red_bits(), green_bits(), blue_bits(), alpha_bits(),
                        coder2.red_bits(), coder2.green_bits(), coder2.blue_bits(), coder2.alpha_bits());
            }

            template<typename P2, typename CODER2>
            static
            void convert(const P &input, P2 &output, const PixelCoder<P2, CODER2> &coder2) {
                color_t input_decoded{};
                decode(input, input_decoded);
                coder2.encode(input_decoded, output);
            }

            inline const char *format() {
                return IMPL::format();
            };

        };

    }
}