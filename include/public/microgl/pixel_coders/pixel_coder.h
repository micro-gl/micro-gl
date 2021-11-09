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

#include "../color.h"
#include "../traits.h"

namespace microgl {
    namespace coder {

        /**
         * encode intensity into a pixel with a pixel coder
         *
         * @tparam number underlying intensity number type
         * @tparam Coder the pixel coder type to use
         *
         * @param input input intensity
         * @param output output pixel
         * @param $coder the coder reference
         */
        template <typename number, class Coder>
        void inline
        encode(const intensity<number> &input, typename Coder::pixel &output,
               const Coder & $coder) {
            color_t int_color{};
            microgl::convert_intensity_to_color<number, typename Coder::rgba>(input, int_color);
            $coder.encode(int_color, output);
        }

        /**
         * decode pixel into intensity using a coder
         *
         * @tparam number underlying intensity number type
         * @tparam Coder the pixel coder type to use
         *
         * @param input input pixel
         * @param output output intensity
         * @param $coder the coder reference
         */
        template <typename number, class Coder>
        void decode(const typename Coder::pixel &input, intensity<number> &output,
                    const Coder & $coder) {
            color_t int_color{};
            $coder.decode(input, int_color);
            microgl::convert_color_to_intensity<number, typename Coder::rgba>(int_color, output);
        }

        /**
         * a base pixel coder that can boost your pixel coder with extra utilities.
         * inheritance is via crpt design pattern. this is a container for your
         * implementation, that adds more utility methods and route methods to your
         * implementation. this is done at compile-time.
         *
         * @tparam pixel_ the pixel storage type
         * @tparam rgba_ the {rgba_t} info object
         * @tparam impl the type of the derived class
         */
        template<typename pixel_, typename rgba_, typename impl>
        class pixel_coder_base : public microgl::traits::crpt<impl> {
        public:
            using pixel= pixel_;
            using rgba= rgba_;

            pixel_coder_base()=default;
            ~pixel_coder_base()=default;

            /**
             * this will invoke encode at your derived class
             *
             * @param input input color
             * @param output output pixel
             */
            void encode(const color_t &input, pixel &output) const {
                this->derived().encode(input, output);
            }
            /**
             * this will invoke decode at your derived class
             *
             * @param input input pixel
             * @param output output color
             */
            void decode(const pixel &input, color_t &output) const {
                this->derived().decode(input, output);
            }

            /**
             * encode intensity to pixel of the coder
             *
             * @tparam number underlying intensity number type
             *
             * @param input input intensity
             * @param output output pixel
             */
            template <typename number>
            void encode(const intensity<number> &input, pixel &output) const {
                coder::encode<number>(input, output, *this);
            }

            /**
             * decode pixel into intensity
             *
             * @tparam number underlying intensity number type
             *
             * @param input input pixel
             * @param output output intensity
             */
            template <typename number>
            void decode(const pixel &input, intensity<number> &output) const {
                coder::decode<number>(input, output, *this);
            }

            /**
             * convert intensity to color of space defined but this coder
             *
             * @tparam number underlying intensity number type
             * @param input intensity
             * @param output color
             */
            template <typename number>
            void convert(const intensity<number> &input, color_t &output) const {
                microgl::convert_intensity_to_color<number, rgba>(input, output);
            }

            /**
             * convert color to intensity
             *
             * @tparam number underlying intensity number type
             *
             * @param input color
             * @param output intensity
             */
            template <typename number>
            void convert(const color_t &input, intensity<number> &output) const {
                microgl::convert_color_to_intensity<number, rgba>(input, output);
            }

            /**
             * convert a color of this coder space into a space of another coder
             *
             * @tparam CODER2 the other coder
             *
             * @param input color
             * @param output color output
             */
            template<typename CODER2>
            void convert(const color_t &input, color_t &output) const {
                // convert input color of my space into a color in coder2 space
                microgl::convert_color<rgba, typename CODER2::rgba>(input, output);
            }

            /**
             * convert a pixel from this coder to a pixel of another coder
             *
             * @tparam CODER2 the other coder
             *
             * @param input input pixel of my coder
             * @param output the output pixel
             * @param coder2 the other coder reference
             */
            template<typename CODER2>
            void convert(const pixel &input, typename CODER2::pixel &output, const CODER2 &coder2) const {
                color_t input_decoded{}, output_converted;
                decode(input, input_decoded);
                convert<CODER2>(input_decoded, output_converted);
                coder2.encode(output_converted, output);
            }
        };
    }
}