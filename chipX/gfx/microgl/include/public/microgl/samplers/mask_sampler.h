#pragma once

#include <microgl/sampler.h>
#include <microgl/masks.h>

namespace microgl {
    extern channel coder::convert_channel_correct(channel input, bits input_bits, bits output_bits);

    namespace sampling {
        template<masks::chrome_mode chrome, class sampler_from, class sampler_mask>
        class mask_sampler : public sampler<sampler_from::r,sampler_from::g,sampler_from::b,sampler_from::a, mask_sampler<chrome, sampler_from, sampler_mask>> {
                using base= sampler<sampler_from::r,sampler_from::g,sampler_from::b,sampler_from::a, mask_sampler<chrome, sampler_from, sampler_mask>>;
                using cr = masks::chrome_mode;
        public:
            sampler_from _s_from;
            sampler_mask _s_mask;
            static constexpr uint8_t alpha_bits= sampler_from::a!=0 ? sampler_from::a : 8;
            static constexpr uint8_t max_alpha_value=(uint16_t(1)<<alpha_bits) - 1;
            static constexpr bool r_test=((chrome==cr::red_channel || chrome==cr::red_channel_inverted) && alpha_bits==sampler_mask::r);
            static constexpr bool g_test=((chrome==cr::green_channel || chrome==cr::green_channel_inverted) && alpha_bits==sampler_mask::g);
            static constexpr bool b_test=((chrome==cr::blue_channel || chrome==cr::blue_channel_inverted) && alpha_bits==sampler_mask::b);
            static constexpr bool a_test=((chrome==cr::alpha_channel || chrome==cr::alpha_channel_inverted) && alpha_bits==sampler_mask::a);
            static constexpr bool not_requires_conversion=r_test||g_test||b_test||a_test;
            static constexpr bool is_inverted=chrome==cr::red_channel_inverted || chrome==cr::green_channel_inverted ||
                    chrome==cr::blue_channel_inverted || chrome==cr::alpha_channel_inverted;

            mask_sampler(const sampler_from & from,
                          const sampler_mask & mask) :
                    _s_from{from}, _s_mask{mask} {
            }

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                color_t color_main{}, mask_color{};
                _s_from.sample(u, v, bits, color_main);
                _s_mask.sample(u, v, bits, mask_color);
                uint8_t alpha;

                switch (chrome) {
                    case masks::chrome_mode::red_channel:
                    case masks::chrome_mode::red_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.r;
                        else {
                            alpha = coder::convert_channel_correct(mask_color.r,
                                                                   _s_mask.r,
                                                                   alpha_bits);
                        }
                        break;
                    case masks::chrome_mode::green_channel:
                    case masks::chrome_mode::green_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.g;
                        else {
                            alpha = coder::convert_channel_correct(mask_color.g,
                                                                   _s_mask.g,
                                                                   alpha_bits);
                        }
                        break;
                    case masks::chrome_mode::blue_channel:
                    case masks::chrome_mode::blue_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.b;
                        else {
                            alpha = coder::convert_channel_correct(mask_color.b,
                                                                   _s_mask.b,
                                                                   alpha_bits);
                        }
                        break;
                    case masks::chrome_mode::alpha_channel:
                    case masks::chrome_mode::alpha_channel_inverted:
                        if (not_requires_conversion) alpha=mask_color.a;
                        else {
                            alpha = coder::convert_channel_correct(mask_color.a,
                                                                   _s_mask.a,
                                                                   alpha_bits);
                        }
                        break;
                }

                if(is_inverted) alpha=max_alpha_value-alpha;
                // we only copy channel values and not bit information
                output.r=color_main.r;
                output.g=color_main.g;
                output.b=color_main.b;
                output.a=(uint16_t(color_main.a)*alpha)>>alpha_bits;
                output.a_bits=alpha_bits;
            }

        };

    }
}