#pragma once

#include <microgl/intensity.h>
#include <microgl/convert_channel.h>

namespace microgl {
    namespace color {
        using namespace microgl::traits;
        using channel = unsigned char;
        using bits = unsigned char;

        struct color_t {
            color_t(channel $r=0, channel $g=0, channel $b=0, channel $a=255) :
                    r{$r}, g{$g}, b{$b}, a{$a} {};
            color_t(const color_t &val) :
                        color_t(val.r, val.g, val.b, val.a){};
            channel r=0, g=0, b=0, a=0;
        };

        template<typename rgba_from, typename rgba_to>
        void convert_color(const color_t &input, color_t &output) {
            output.r = convert_channel_correct<rgba_from::r, rgba_to::r>(input.r);
            output.g = convert_channel_correct<rgba_from::g, rgba_to::g>(input.g);
            output.b = convert_channel_correct<rgba_from::b, rgba_to::b>(input.b);
            output.a = convert_channel_correct<rgba_from::a, rgba_to::a>(input.a);
        }

        template <typename number, typename rgba>
        void convert_color(const intensity<number> &input, color_t &output) {
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
