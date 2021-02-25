#pragma once

#include <microgl/PixelCoder.h>

namespace microgl {
    namespace coder {

        /**
         * a coder that rescales it's rgba values. it is advised to use a custom one,
         * that can be done more efficiently or make sure you data almost always requires
         * the same pixel coding
         *
         * @tparam from a pixel_coder
         * @tparam R
         * @tparam G
         * @tparam B
         * @tparam A
         */
        template<class from, channel R, channel G, channel B, channel A>
        class coder_rgba :
                public PixelCoder<typename from::Pixel, R, G, B, A,
                        coder_rgba<from, R, G, B, A>> {

        private:
            using base = PixelCoder<typename from::Pixel, R, G, B, A,
                    coder_rgba<from, R, G, B, A>>;
            using pixel_from = typename from::Pixel;
            from _coder_from;
        public:
            using base::decode;
            using base::encode;

            coder_rgba() = default;

            inline void encode(const color_t &input, pixel_from &output) const {
                color_t converted_color{};
                coder::convert_color(
                        input, converted_color,
                        R, G, B, A,
                        from::r(), from::g(), from::b(), from::a());
                _coder_from.encode(converted_color, output);
            }

            inline void decode(const pixel_from &input, color_t &output) const {
                color_t converted_color;
                _coder_from.decode(input, converted_color);
                coder::convert_color(
                        converted_color, output,
                        from::r(), from::g(), from::b(), from::a(),
                        R, G, B, A);
            };

        };

    }

}