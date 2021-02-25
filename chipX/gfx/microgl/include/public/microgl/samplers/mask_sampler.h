#pragma once

#include <microgl/sampler.h>
#include <microgl/masks.h>

namespace microgl {
    extern channel coder::convert_channel_correct(channel input, bits input_bits, bits output_bits);

    namespace sampling {
        template<class sampler_from, class sampler_mask, masks::chrome_mode chrome>
        class mask_sampler : public sampler<mask_sampler<sampler_from, sampler_mask, chrome>> {
            using base= sampler<mask_sampler<sampler_from, sampler_mask, chrome>>;

        public:
            sampler_from _s_from;
            sampler_mask _s_mask;
            static constexpr uint8_t alpha_bits= 8 , max_alpha_value=(uint16_t(1)<<alpha_bits) - 1;


            mask_sampler(const sampler_from & from,
                          const sampler_mask & mask) :
                    _s_from{from}, _s_mask{mask} {
//                _bits_alpha_mask = mask.alpha_bits();
//                _bits_alpha_main = from.alpha_bits();
//                alpha_bits = 8;//_s_from.alpha_bits() ? _s_from.alpha_bits() : 8;
//                max_alpha_value = (uint16_t(1)<<alpha_bits) - 1;

            }

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                color_t color_main{}, mask_color{};
                _s_from.sample(u, v, bits, color_main);
                _s_mask.sample(u, v, bits, mask_color);

//                const uint8_t alpha_bits = _s_from.alpha_bits() ? _s_from.alpha_bits() : 8;
//                const uint8_t max_alpha_value = (uint16_t(1)<<alpha_bits) - 1;
                uint8_t a;

                switch (chrome) {
                    case masks::chrome_mode::red_channel:
                        a=mask_color.r;break;
                        a = coder::convert_channel_correct(mask_color.r,
                                                           _s_mask.red_bits(),
                                                           alpha_bits);
                        break;
                    case masks::chrome_mode::red_channel_inverted:
                        a = max_alpha_value - coder::convert_channel_correct(mask_color.r,
                                                                             _s_mask.red_bits(),
                                                                             alpha_bits);
                        break;
                    case masks::chrome_mode::alpha_channel:
                        a = coder::convert_channel_correct(mask_color.a,
                                                           _s_mask.alpha_bits(),
                                                           alpha_bits);
                        break;
                    case masks::chrome_mode::alpha_channel_inverted:
                        a = max_alpha_value - coder::convert_channel_correct(mask_color.a,
                                                                             _s_mask.alpha_bits(),
                                                                             alpha_bits);
                        break;
                    case masks::chrome_mode::green_channel:
                        a = coder::convert_channel_correct(mask_color.g,
                                                           _s_mask.green_bits(),
                                                           alpha_bits);
                        break;
                    case masks::chrome_mode::green_channel_inverted:
                        a = max_alpha_value - coder::convert_channel_correct(mask_color.g,
                                                                             _s_mask.green_bits(),
                                                                             alpha_bits);
                        break;
                    case masks::chrome_mode::blue_channel:
                        a = coder::convert_channel_correct(mask_color.b,
                                                           _s_mask.blue_bits(),
                                                           alpha_bits);
                        break;
                    case masks::chrome_mode::blue_channel_inverted:
                        a = max_alpha_value - coder::convert_channel_correct(mask_color.b,
                                                                             _s_mask.blue_bits(),
                                                                             alpha_bits);
                        break;
                }
                // we only copy channel values and not bit information
                output.r=color_main.r;
                output.g=color_main.g;
                output.b=color_main.b;
                output.a=uint16_t(color_main.a*a)>>alpha_bits;
                output.a_bits=alpha_bits;
            }

        };

    }
}