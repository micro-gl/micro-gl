#pragma once

#include <types.h>

namespace fontium {
        struct color_t {
            color_t(channel $r, channel $g, channel $b, channel $a,
                    bits $r_bits, bits $g_bits, bits $b_bits, bits $a_bits) :
                    r{$r}, g{$g}, b{$b}, a{$a}, r_bits{$r_bits}, g_bits{$g_bits},
                    b_bits{$b_bits}, a_bits{$a_bits} {};
            color_t(const color_t &val) :
                    color_t(val.r, val.g, val.b, val.a, val.r_bits, val.g_bits, val.b_bits,val.a_bits) {};
            color_t()= default;
            channel r=0, g=0, b=0, a=0;
            bits r_bits=8, g_bits=8, b_bits=8, a_bits=8;
            bits bits_rgba() const { return r_bits+g_bits+b_bits+a_bits; }
            bits bits_gba() const { return g_bits+b_bits+a_bits; }
            bits bits_ba() const { return b_bits+a_bits; }
            bits bits_a() const { return a_bits; }
        };
}
