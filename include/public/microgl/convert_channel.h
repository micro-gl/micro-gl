#pragma once

#include <microgl/micro_gl_traits.h>

namespace microgl {
    using namespace microgl::traits;

    template<uint8_t bits_from, uint8_t bits_to>
    typename unsigned_type_infer<bits_to>::type convert_channel_correct(const typename unsigned_type_infer<bits_from>::type &input) {
        if (bits_from == bits_to) return input;

        using type_bits_from = typename unsigned_type_infer<bits_from>::type;
        using type_bits_to = typename unsigned_type_infer<bits_to>::type;

        constexpr type_bits_from max_input= (1u<<bits_from)-1;
        constexpr type_bits_to max_output= (1u<<bits_to)-1;
        constexpr uint8_t p = bits_from + 12;
        constexpr uint8_t bits_used = p + 1 - bits_from;
        using type_bits_used = typename unsigned_type_infer<bits_used>::type;
        using type_bits_p = typename unsigned_type_infer<p>::type;
        constexpr type_bits_p one = (type_bits_p(1u)<<p);
        constexpr type_bits_p half = one>>1;
        constexpr type_bits_used multiplier = one/max_input;
        constexpr uint8_t overall_bits = bits_from+bits_to+bits_used;
//            constexpr bool overflow = false;
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

    template<uint8_t bits>
    typename unsigned_type_infer<bits>::type mul_channels_correct(
            const typename unsigned_type_infer<bits>::type & a,
            const typename unsigned_type_infer<bits>::type & b) {
//        if (bits_from == bits_to) return input;

        // blinn's method
        if(bits==8) {
            return (uint16_t(a)*b*257)>>16;
        }

        using type_bits = typename unsigned_type_infer<bits>::type;

        constexpr type_bits max_value= (1u<<bits)-1;
//        constexpr type_bits_to max_output= (1u<<bits_to)-1;
        constexpr uint8_t p = bits + 12;
        constexpr uint8_t bits_used = p + 1 - bits;
        using type_bits_used = typename unsigned_type_infer<bits_used>::type;
        using type_bits_p = typename unsigned_type_infer<p>::type;
        constexpr type_bits_p one = (type_bits_p(1u)<<p);
        constexpr type_bits_p half = one>>1;
        constexpr type_bits_used multiplier = one/max_value;
        constexpr uint8_t overall_bits = bits+bits+bits_used;
        using type_bits_overall = typename unsigned_type_infer<overall_bits+1>::type;

//            constexpr bool overflow = false;
//            constexpr bool overflow = true;
        constexpr bool overflow = overall_bits>=32;
        type_bits result = 0;

        if(overflow)
            result = ((uint32_t)a*b)/max_value;
        else {
            result = ((type_bits_overall)a*b*multiplier + half)>>p;
        }

        return result;
    }

    template<uint8_t bits>
    typename unsigned_type_infer<bits>::type mc(
            const typename unsigned_type_infer<bits>::type & a,
            const typename unsigned_type_infer<bits>::type & b) {
        return mul_channels_correct<bits>(a,b);
    }
}