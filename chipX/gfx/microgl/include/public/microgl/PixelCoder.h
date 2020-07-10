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

        template<typename P, channel R_BITS, channel G_BITS, channel B_BITS, channel A_BITS, typename IMPL>
        class PixelCoder : public crpt<IMPL> {
        public:
            using Pixel=P;

            void encode(const color_t &input, P &output) const {
                this->derived().encode(input, output);
            }
            void decode(const P &input, color_t &output) const {
                this->derived().decode(input, output);
            }

            template <typename number>
            void encode(const intensity<number> &input, P &output) const {
                color_t int_color{};
                convert_color(input, int_color, R_BITS, G_BITS, B_BITS, A_BITS);
                encode(int_color, output);
            }

            template <typename number>
            void decode(const P &input, intensity<number> &output) const {
                color_t int_color{};
                decode(input, int_color);
                convert_color(int_color, output, R_BITS, G_BITS, B_BITS, A_BITS);
            }

            static constexpr channel red_bits() { return R_BITS; }
            static constexpr channel green_bits() { return G_BITS; }
            static constexpr channel blue_bits() { return B_BITS; }
            static constexpr channel alpha_bits() { return A_BITS; }

            static
            void update_channel_bit(color_t &color) {
                color.r_bits = R_BITS;
                color.g_bits = G_BITS;
                color.b_bits = B_BITS;
                color.a_bits = A_BITS;
            }

            template <typename number>
            void convert(const intensity<number> &input, color_t &output) const {
                convert_color(input, output, red_bits(), green_bits(), blue_bits(), alpha_bits());
            }

            template <typename number>
            void convert(const color_t &input, intensity<number> &output) const {
                convert_color(input, output, red_bits(), green_bits(), blue_bits(), alpha_bits());
            }

            template<typename CODER2>
            void convert(const color_t &input, color_t &output, const CODER2 &coder2) const {
                // convert input color of my space into a color in coder2 space
                coder::convert_color(
                        input, output,
                        red_bits(), green_bits(), blue_bits(), alpha_bits(),
                        coder2.red_bits(), coder2.green_bits(), coder2.blue_bits(), coder2.alpha_bits());
            }

            template<typename CODER2>
            void convert(const P &input, typename CODER2::Pixel &output, const CODER2 &coder2) const {
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