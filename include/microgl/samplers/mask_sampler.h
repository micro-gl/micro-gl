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

#include <microgl/masks.h>

namespace microgl {

    namespace sampling {

        /**
         * a sampler that masks a sampler with another sampler
         *
         * @tparam chrome the chrome channel config for mask sampler
         * @tparam sampler_from a sampler you want to mask
         * @tparam sampler_mask a sampler that acts as a mask
         * @tparam alpha_fallback this sampler inherits the alpha bits of sampler_from,
         *                        and in case it doesn't have an alpha channel (rgba::a=0),
         *                        we can make an alpha channel with `alpha_fallback` bits
         */
        template<masks::chrome_mode chrome, class sampler_from, class sampler_mask, microgl::ints::uint8_t alpha_fallback=8>
        struct mask_sampler {
            using cr = masks::chrome_mode;
            using rgba = rgba_dangling_a<sampler_rgba<sampler_from>, alpha_fallback>;

        private:
            sampler_from _s_from;
            sampler_mask _s_mask;
            static constexpr microgl::ints::uint8_t alpha_bits= rgba::a;
            static constexpr microgl::ints::uint8_t max_alpha_value=(microgl::ints::uint16_t(1)<<alpha_bits) - 1;
            static constexpr bool r_test=((chrome==cr::red_channel || chrome==cr::red_channel_inverted) &&
                    alpha_bits==sampler_mask::rgba::r);
            static constexpr bool g_test=((chrome==cr::green_channel || chrome==cr::green_channel_inverted) &&
                    alpha_bits==sampler_mask::rgba::g);
            static constexpr bool b_test=((chrome==cr::blue_channel || chrome==cr::blue_channel_inverted) &&
                    alpha_bits==sampler_mask::rgba::b);
            static constexpr bool a_test=((chrome==cr::alpha_channel || chrome==cr::alpha_channel_inverted) &&
                    alpha_bits==sampler_mask::rgba::a);
            static constexpr bool not_requires_conversion=r_test||g_test||b_test||a_test;
            static constexpr bool is_inverted=chrome==cr::red_channel_inverted || chrome==cr::green_channel_inverted ||
                    chrome==cr::blue_channel_inverted || chrome==cr::alpha_channel_inverted;

        public:
            mask_sampler(const sampler_from & from,
                         const sampler_mask & mask) :
                            _s_from{from}, _s_mask{mask} {
            }

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                color_t color_main{}, mask_color{};
                _s_from.sample(u, v, bits, color_main);
                _s_mask.sample(u, v, bits, mask_color);
                microgl::ints::uint8_t alpha;

                switch (chrome) {
                    case masks::chrome_mode::red_channel:
                    case masks::chrome_mode::red_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.r;
                        else {
                            alpha = microgl::convert_channel_correct<sampler_mask::rgba::r, alpha_bits>(mask_color.r);
                        }
                        break;
                    case masks::chrome_mode::green_channel:
                    case masks::chrome_mode::green_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.g;
                        else {
                            alpha = microgl::convert_channel_correct<sampler_mask::rgba::g, alpha_bits>(mask_color.g);
                        }
                        break;
                    case masks::chrome_mode::blue_channel:
                    case masks::chrome_mode::blue_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.b;
                        else {
                            alpha = microgl::convert_channel_correct<sampler_mask::rgba::b, alpha_bits>(mask_color.b);
                        }
                        break;
                    case masks::chrome_mode::alpha_channel:
                    case masks::chrome_mode::alpha_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.a;
                        else {
                            alpha = microgl::convert_channel_correct<sampler_mask::rgba::a, alpha_bits>(mask_color.a);
                        }
                        break;
                }

                if(is_inverted) alpha=max_alpha_value-alpha;
                // we only copy channel values and not bit information
                output.r=color_main.r;
                output.g=color_main.g;
                output.b=color_main.b;
                // todo: add a precise division
                output.a=(microgl::ints::uint16_t(color_main.a)*alpha)>>alpha_bits;
            }

        };

    }
}