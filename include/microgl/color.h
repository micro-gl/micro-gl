/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "./stdint.h"

namespace microgl {
    using namespace microgl::traits;
    using bits = unsigned char;
    using channel_t = unsigned char;

    /**
     * generic static rgba info container
     *
     * @tparam R red bits
     * @tparam G green bits
     * @tparam B blue bits
     * @tparam A alpha bits
     */
    template<microgl::ints::uint8_t R, microgl::ints::uint8_t G, microgl::ints::uint8_t B, microgl::ints::uint8_t A>
    struct rgba_t {
        static constexpr microgl::ints::uint8_t r = R;
        static constexpr microgl::ints::uint8_t g = G;
        static constexpr microgl::ints::uint8_t b = B;
        static constexpr microgl::ints::uint8_t a = A;

        rgba_t() = delete;
        rgba_t(const rgba_t &) = delete;
        rgba_t(rgba_t &&) = delete;
        rgba_t& operator=(const rgba_t &) = delete;
        rgba_t& operator=(rgba_t &&) = delete;
        ~rgba_t() = delete;
    };

    /**
     * given an rgba object with zero alpha, create a new type with alpha_fallback
     */
    template<class rgba, microgl::ints::uint8_t alpha_fallback=8>
    using rgba_dangling_a = rgba_t<rgba::r, rgba::g, rgba::b, rgba::a != 0 ? rgba::a : alpha_fallback>;

    /**
     * statically assert both rgb values agree
     * @tparam rgba_1 first
     * @tparam rgba_2 second
     */
    template<typename rgba_1, typename rgba_2, bool mute=false>
    void static_assert_rgb() {
        static_assert(mute || rgba_1::r==rgba_2::r, "R channel bits is not equal");
        static_assert(mute || rgba_1::g==rgba_2::g, "G channel bits is not equal");
        static_assert(mute || rgba_1::b==rgba_2::b, "B channel bits is not equal");
    }

    /**
     * statically assert both rgba values agree
     * @tparam rgba_1 first
     * @tparam rgba_2 second
     */
    template<class rgba_1, class rgba_2, bool mute=false>
    void static_assert_rgba() {
        static_assert_rgb<rgba_1, rgba_2, mute>();
        static_assert(mute || rgba_1::a!=rgba_2::a, "Alpha channel bits is not equal");
    }

    struct color_t {
        color_t(channel_t $r=0, channel_t $g=0, channel_t $b=0, channel_t $a=255) :
                r{$r}, g{$g}, b{$b}, a{$a} {};
        color_t & operator=(const color_t & rhs) {
            r=rhs.r; g=rhs.g; b=rhs.b; a=rhs.a;
            return (*this);
        }
        channel_t r, g, b, a;
    };

    template <typename number=float>
    struct intensity {
        intensity(const number &r=number(0),
                  const number &g=number(0),
                  const number &b=number(0),
                  const number &a=number(1)) :
                r{r}, g{g}, b{b}, a{a} {}
        number r, g, b, a;
    };

    template<microgl::ints::uint8_t bits_from, microgl::ints::uint8_t bits_to>
    inline microgl::ints::uint_t<bits_to> convert_channel_correct(const microgl::ints::uint_t<bits_from> &input) {
        if (bits_from == bits_to) return input;
        if(bits_to==0) return 0;
        if(bits_from==0) return 0;

        using uint_bits_from = microgl::ints::uint_t<bits_from>;
        using uint_bits_to = microgl::ints::uint_t<bits_to>;

        constexpr uint_bits_from max_input= (1u << bits_from) - 1;
        constexpr uint_bits_to max_output= (1u << bits_to) - 1;
        constexpr microgl::ints::uint8_t p = bits_from + 12;
        constexpr microgl::ints::uint8_t bits_used = p + 1 - bits_from;
        using type_bits_used = microgl::ints::uint_t<bits_used>;
        using type_bits_p = microgl::ints::uint_t<p>;
        constexpr type_bits_p one = (type_bits_p(1u)<<p);
        constexpr type_bits_p half = one>>1;
        // the condition max_input!=0 is just to prevent the compiler from detecting
        // zero division, in which case it will unqualify the constexpr with an error
        constexpr type_bits_used multiplier = max_input!=0 ? one/max_input : 1;
        constexpr microgl::ints::uint8_t overall_bits = bits_from+bits_to+bits_used;
//            constexpr bool overflow = false;
//            constexpr bool overflow = true;
        constexpr bool overflow = overall_bits>=32;
        microgl::ints::uint32_t result = 0;

        if(overflow) result = ((microgl::ints::uint32_t)input*max_output)/max_input;
        else result = ((microgl::ints::uint32_t)input*max_output*multiplier + half)>>p;
        return result;
    }

    template<microgl::ints::uint8_t bits>
    inline microgl::ints::uint_t<bits> mul_channels_correct(
            const microgl::ints::uint_t<bits> & a,
            const microgl::ints::uint_t<bits> & b) {

        // blinn's method
        if(bits==8) return (microgl::ints::uint32_t (a)*b*257 + 257)>>16;
        else if(bits==0) return 0;

        using type_bits = microgl::ints::uint_t<bits>;
        constexpr type_bits max_value= (1u<<bits)-1;
        constexpr microgl::ints::uint8_t p = bits + 12;
        constexpr microgl::ints::uint8_t bits_used = p + 1 - bits;
        using type_bits_used = microgl::ints::uint_t<bits_used>;
        using type_bits_p = microgl::ints::uint_t<p>;
        constexpr type_bits_p one = (type_bits_p(1u)<<p);
        constexpr type_bits_p half = one>>1;
        constexpr type_bits_used multiplier = one/max_value;
        constexpr microgl::ints::uint8_t overall_bits = bits+bits+bits_used;
        using type_bits_overall = microgl::ints::uint_t<overall_bits+1>;

//            constexpr bool overflow = false;
//            constexpr bool overflow = true;
        constexpr bool overflow = overall_bits>=32;
        type_bits result = 0;

        if(overflow) result = ((microgl::ints::uint32_t)a*b)/max_value;
        else result = ((type_bits_overall)a*b*multiplier + half)>>p;
        return result;
    }

    template<microgl::ints::uint8_t bits>
    inline microgl::ints::uint_t<bits> mc(
            const microgl::ints::uint_t<bits> & a,
            const microgl::ints::uint_t<bits> & b) {
        return mul_channels_correct<bits>(a,b);
    }

    template<microgl::ints::uint8_t bits_a, microgl::ints::uint8_t bits_b, microgl::ints::uint_t<bits_b> b>
    inline microgl::ints::uint_t<bits_a-bits_b+1> div(
            const microgl::ints::uint_t<bits_a> & a) {
        using uint_b = microgl::ints::uint_t<bits_b>;
        constexpr microgl::ints::uint8_t p = bits_b + 12;
        constexpr microgl::ints::uint8_t bits_used_multiplier = p + 1 - bits_b;
        using uint_multiplier = microgl::ints::uint_t<bits_used_multiplier>;
        using uint_p = microgl::ints::uint_t<p>;
        using uint_result = microgl::ints::uint_t<bits_a-bits_b+1>;
        constexpr uint_p one = (uint_p(1u)<<p);
        constexpr uint_p half = one>>1;
        constexpr uint_multiplier multiplier = one / b;
        constexpr microgl::ints::uint8_t overall_bits = bits_a + bits_used_multiplier;
        using type_bits_overall = microgl::ints::uint_t<overall_bits+1>;
        constexpr bool overflow = overall_bits>=32;
//            constexpr bool overflow = false;

        uint_result result = 0;

        if(overflow) result = ((microgl::ints::uint32_t)a)/b;
        else result = ((type_bits_overall)a*multiplier + half)>>p;
        return result;
    }

    /**
     * convert a color of one depth space into another
     *
     * @tparam rgba_from the rgba_t info of the input color
     * @tparam rgba_to the rgba_t info of the output color
     * @param input input color
     * @param output output color
     */
    template<typename rgba_from, typename rgba_to>
    void convert_color(const color_t &input, color_t &output) {
        output.r = convert_channel_correct<rgba_from::r, rgba_to::r>(input.r);
        output.g = convert_channel_correct<rgba_from::g, rgba_to::g>(input.g);
        output.b = convert_channel_correct<rgba_from::b, rgba_to::b>(input.b);
        output.a = convert_channel_correct<rgba_from::a, rgba_to::a>(input.a);
    }

    /**
     * convert an intensity to color, example :
     *
     * Example: intensity<float>(0.5, 1.0, 1.0f) -> color_t{128, 255, 255}
     *
     * @tparam number the underlying type of the intensity
     * @tparam rgba the rgba_t info we desire to quantize
     * @param input the input intensity
     * @param output output color
     */
    template <typename number, typename rgba>
    void convert_intensity_to_color(const intensity<number> &input, color_t &output) {
        output.r = channel_t(number((1u << rgba::r) - 1) * input.r);
        output.g = channel_t(number((1u << rgba::g) - 1) * input.g);
        output.b = channel_t(number((1u << rgba::b) - 1) * input.b);
        output.a = channel_t(number((1u << rgba::a) - 1) * input.a);
    }

    /**
     * convert a color into intensity object
     *
     * Example: color_t{128, 255, 255} -> intensity<float>(0.5, 1.0, 1.0f)
     *
     * @tparam number number the underlying type of the intensity
     * @tparam rgba the rgba_t info of the color
     * @param input input color
     * @param output output intensity
     */
    template <typename number, typename rgba>
    void convert_color_to_intensity(const color_t &input, intensity<number> &output) {
        output.r = number(input.r)/number((1u << rgba::r) - 1);
        output.g = number(input.g)/number((1u << rgba::g) - 1);
        output.b = number(input.b)/number((1u << rgba::b) - 1);
        output.a = number(input.a)/number((1u << rgba::a) - 1);
    }
}
