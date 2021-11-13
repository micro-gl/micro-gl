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

#include <microgl/color.h>

namespace microgl {
    namespace coder {

        /**
         * a coder that rescales it's rgba_t values. it is advised to use a custom one,
         * that can be done more efficiently or make sure you data almost always requires
         * the same pixel coding. this can be sped up with a lookup table. Also, no division occurs
         * as I exchange division with multiplication and shifting at compile time.
         *
         * @tparam from_coder a pixel_coder
         * @tparam rgba_ output {@rgba_t} info interface
         */
        template<class from_coder, typename rgba_>
        struct coder_converter_rgba {
            using rgba = rgba_;
            using pixel = typename from_coder::pixel;

        private:
            using pixel_from = typename from_coder::pixel;
            from_coder _coder_from;
        public:

            coder_converter_rgba() = default;

            inline void encode(const color_t &input, pixel_from &output) const {
                color_t converted_color{};
                microgl::convert_color<rgba , typename from_coder::rgba>(
                        input, converted_color);
                _coder_from.encode(converted_color, output);
            }

            inline void decode(const pixel_from &input, color_t &output) const {
                color_t converted_color;
                _coder_from.decode(input, converted_color);
                microgl::convert_color<typename from_coder::rgba, rgba>(
                        converted_color, output);
            };

        };

    }

}