#pragma once

#include <microgl/intensity.h>
#include <microgl/micro_gl_traits.h>

namespace microgl {
    namespace color {
        using namespace microgl::traits;
        using channel = unsigned char;
        using bits = unsigned char;

        struct color_t {
            color_t(channel $r=0, channel $g=0, channel $b=0, channel $a=255,
                    bits $r_bits=8, bits $g_bits=8, bits $b_bits=8, bits $a_bits=8) :
                    r{$r}, g{$g}, b{$b}, a{$a}, r_bits{$r_bits}, g_bits{$g_bits}, b_bits{$b_bits}, a_bits{$a_bits} {};
            color_t(const color_t &val) :
                        color_t(val.r, val.g, val.b, val.a, val.r_bits, val.g_bits, val.b_bits,val.a_bits) {};
            channel r=0, g=0, b=0, a=0;
            bits r_bits=8, g_bits=8, b_bits=8, a_bits=8;
        };

        template<bits bits_from, bits bits_to>
        typename unsigned_type_infer<bits_to>::type convert_channel_correct(typename unsigned_type_infer<bits_from>::type input) {
            if (bits_from == bits_to) return input;

            using type_bits_from = typename unsigned_type_infer<bits_from>::type;
            using type_bits_to = typename unsigned_type_infer<bits_to>::type;

            constexpr type_bits_from max_input= (1u<<bits_from)-1;
            constexpr type_bits_to max_output= (1u<<bits_to)-1;
            constexpr uint8_t p = bits_from + 12;
            constexpr uint8_t bits_used = p + 1 - bits_from;
            using type_bits_used = typename unsigned_type_infer<bits_used>::type;
            using type_bits_p = typename unsigned_type_infer<p>::type;
            constexpr type_bits_p one = (type_bits_used(1u)<<p);
            constexpr type_bits_p half = one>>1;
            constexpr type_bits_used multiplier = one/max_input;
            constexpr uint8_t overall_bits = bits_from+bits_to+bits_used;
//            constexpr bool overflow = true;
            constexpr bool overflow = overall_bits>=32;
            uint32_t result = 0;

            if(overflow)
                result = ((uint32_t)input*max_output)/max_input;
            else {
                result = ((uint32_t)input*max_output*multiplier + half)>>p;
            }

            return result;
        }

        template<typename rgba_from, typename rgba_to>
        void convert_color(const color_t &input, color_t &output) {
            output.r = convert_channel_correct<rgba_from::r, rgba_to::r>(input.r);
            output.g = convert_channel_correct<rgba_from::g, rgba_to::g>(input.g);
            output.b = convert_channel_correct<rgba_from::b, rgba_to::b>(input.b);
            output.a = convert_channel_correct<rgba_from::a, rgba_to::a>(input.a);
        }

        template <typename number, typename rgba>
        void convert_color(const intensity<number> &input, color_t &output,
                           const bits &output_r_bits, const bits &output_g_bits,
                           const bits &output_b_bits, const bits &output_a_bits) {
            output.r = channel(input.r * number((1u << rgba::r) - 1));
            output.g = channel(input.g * number((1u << rgba::g) - 1));
            output.b = channel(input.b * number((1u << rgba::b) - 1));
            output.a = channel(input.a * number((1u << rgba::a) - 1));
        }

        template <typename number, typename rgba>
        void convert_color(const color_t &input, intensity<number> &output) {
            output.r = number(input.r)/number((1u << rgba::r) - 1);
            output.g = number(input.g)/number((1u << rgba::g) - 1);
            output.b = number(input.b)/number((1u << rgba::b) - 1);
            output.a = number(input.a)/number((1u << rgba::a) - 1);
        }

    }

}
