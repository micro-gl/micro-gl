#pragma once

#include <microgl/micro_gl_traits.h>

namespace microgl {
    using namespace microgl::traits;

    template<uint8_t bits_from, uint8_t bits_to>
    inline uint_t<bits_to> convert_channel_correct(const uint_t<bits_from> &input) {
        if (bits_from == bits_to) return input;
        if(bits_to==0) return 0;
        if(bits_from==0) return 0;

        using uint_bits_from = uint_t<bits_from>;
        using uint_bits_to = uint_t<bits_to>;

        constexpr uint_bits_from max_input= (1u << bits_from) - 1;
        constexpr uint_bits_to max_output= (1u << bits_to) - 1;
        constexpr uint8_t p = bits_from + 12;
        constexpr uint8_t bits_used = p + 1 - bits_from;
        using type_bits_used = uint_t<bits_used>;
        using type_bits_p = uint_t<p>;
        constexpr type_bits_p one = (type_bits_p(1u)<<p);
        constexpr type_bits_p half = one>>1;
        // the condition max_input!=0 is just to prevent the compiler from detecting
        // zero division, in which case it will unqualify the constexpr with an error
        constexpr type_bits_used multiplier = max_input!=0 ? one/max_input : 1;
        constexpr uint8_t overall_bits = bits_from+bits_to+bits_used;
//            constexpr bool overflow = false;
//            constexpr bool overflow = true;
        constexpr bool overflow = overall_bits>=32;
        uint32_t result = 0;

        if(overflow) result = ((uint32_t)input*max_output)/max_input;
        else result = ((uint32_t)input*max_output*multiplier + half)>>p;
        return result;
    }

    template<uint8_t bits>
    inline uint_t<bits> mul_channels_correct(
            const uint_t<bits> & a,
            const uint_t<bits> & b) {

        // blinn's method
        if(bits==8) return (uint32_t (a)*b*257 + 257)>>16;
        else if(bits==0) return 0;

        using type_bits = uint_t<bits>;
        constexpr type_bits max_value= (1u<<bits)-1;
        constexpr uint8_t p = bits + 12;
        constexpr uint8_t bits_used = p + 1 - bits;
        using type_bits_used = uint_t<bits_used>;
        using type_bits_p = uint_t<p>;
        constexpr type_bits_p one = (type_bits_p(1u)<<p);
        constexpr type_bits_p half = one>>1;
        constexpr type_bits_used multiplier = one/max_value;
        constexpr uint8_t overall_bits = bits+bits+bits_used;
        using type_bits_overall = uint_t<overall_bits+1>;

//            constexpr bool overflow = false;
//            constexpr bool overflow = true;
        constexpr bool overflow = overall_bits>=32;
        type_bits result = 0;

        if(overflow) result = ((uint32_t)a*b)/max_value;
        else result = ((type_bits_overall)a*b*multiplier + half)>>p;
        return result;
    }

    template<uint8_t bits>
    inline uint_t<bits> mc(
            const uint_t<bits> & a,
            const uint_t<bits> & b) {
        return mul_channels_correct<bits>(a,b);
    }

    template<uint8_t bits_a, uint8_t bits_b, uint_t<bits_b> b>
    inline uint_t<bits_a-bits_b+1> div(
            const uint_t<bits_a> & a) {
        using uint_b = uint_t<bits_b>;
        constexpr uint8_t p = bits_b + 12;
        constexpr uint8_t bits_used_multiplier = p + 1 - bits_b;
        using uint_multiplier = uint_t<bits_used_multiplier>;
        using uint_p = uint_t<p>;
        using uint_result = uint_t<bits_a-bits_b+1>;
        constexpr uint_p one = (uint_p(1u)<<p);
        constexpr uint_p half = one>>1;
        constexpr uint_multiplier multiplier = one / b;
        constexpr uint8_t overall_bits = bits_a + bits_used_multiplier;
        using type_bits_overall = uint_t<overall_bits+1>;
//        constexpr bool overflow = overall_bits>=32;
        constexpr bool overflow = false;

        uint_result result = 0;

        if(overflow) result = ((uint32_t)a)/b;
        else result = ((type_bits_overall)a*multiplier + half)>>p;
        return result;
    }

}