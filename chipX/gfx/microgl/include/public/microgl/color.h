#pragma once

namespace microgl {
    namespace color {
        using channel = unsigned char;
        using bits = unsigned char;

        struct color_t {
            color_t(channel $r=0, channel $g=0, channel $b=0, channel $a=255,
                    bits $r_bits=8, bits $g_bits=8, bits $b_bits=8, bits $a_bits=8) :
                    r{$r}, g{$g}, b{$b}, a{$a}, r_bits{$r_bits}, g_bits{$g_bits}, a_bits{$a_bits} {};
            color_t(const color_t &val) :
                        color_t(val.r, val.g, val.b, val.a, val.r_bits, val.g_bits, val.b_bits,val.a_bits) {};

            channel r=0, g=0, b=0, a=0;
            bits r_bits=8, g_bits=8, b_bits=8, a_bits=8;
        };

        struct color_f_t {
            color_f_t(const float &$r=1.0f, const float &$g=1.0f, const float &$b=1.0f, const float &$a=1.0f) :
                                r{$r}, g{$g}, b{$b}, a{$a} {}
            color_f_t(const color_f_t &val) = default;
            float r, g, b, a;
        };

        class colors {
        public:
            static color_f_t RED;
            static color_f_t YELLOW;
            static color_f_t WHITE;
            static color_f_t GREEN;
            static color_f_t BLUE;
            static color_f_t BLACK;

        };

    }

}
