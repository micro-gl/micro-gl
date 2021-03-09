#pragma once

#include <microgl/pixel_coder.h>

namespace microgl {
    namespace coder {

        /**
         * a coder that rescales it's rgba_t values. it is advised to use a custom one,
         * that can be done more efficiently or make sure you data almost always requires
         * the same pixel coding
         *
         * @tparam from_coder a pixel_coder
         * @tparam rgba_ output {@rgba_t} info interface
         */
        template<class from_coder, typename rgba_>
        class coder_converter_rgba :
                public pixel_coder<typename from_coder::pixel, rgba_, coder_converter_rgba<from_coder, rgba_>> {

        private:
            using base = pixel_coder<typename from_coder::pixel, rgba_, coder_converter_rgba<from_coder, rgba_>>;
            using pixel_from = typename from_coder::pixel;
            from_coder _coder_from;
        public:
            using base::decode;
            using base::encode;

            coder_converter_rgba() = default;

            inline void encode(const color_t &input, pixel_from &output) const {
                color_t converted_color{};
                color::convert_color<base::rgba, from_coder::rgba>(
                        input, converted_color);
                _coder_from.encode(converted_color, output);
            }

            inline void decode(const pixel_from &input, color_t &output) const {
                color_t converted_color;
                _coder_from.decode(input, converted_color);
                color::convert_color<from_coder::rgba, base::rgba>(
                        converted_color, output);
            };

        };

    }

}