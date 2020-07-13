#pragma once

namespace microgl {
    namespace color {
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

    }

}
