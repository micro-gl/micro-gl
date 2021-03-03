#pragma once

#include <microgl/rgba_t.h>
#include <microgl/color.h>
#include <microgl/crpt.h>

namespace microgl {

    namespace coder {

        using namespace microgl::color;
        using namespace microgl::traits;
        using channel = uint8_t;
        using bits = uint8_t;

        template<typename pixel_, typename rgba_, typename impl>
        class pixel_coder : public crpt<impl> {
        public:
            using pixel= pixel_;
            using rgba= rgba_;

            pixel_coder()=default;
            ~pixel_coder()=default;

            void encode(const color_t &input, pixel &output) const {
                this->derived().encode(input, output);
            }
            void decode(const pixel &input, color_t &output) const {
                this->derived().decode(input, output);
            }

            template <typename number>
            void encode(const intensity<number> &input, pixel &output) const {
                color_t int_color{};
                convert_color<number, rgba>(input, int_color);
                encode(int_color, output);
            }

            template <typename number>
            void decode(const pixel &input, intensity<number> &output) const {
                color_t int_color{};
                decode(input, int_color);
                convert_color<number, rgba>(int_color, output);
            }

            template <typename number>
            void convert(const intensity<number> &input, color_t &output) const {
                convert_color<number, rgba>(input, output);
            }

            template <typename number>
            void convert(const color_t &input, intensity<number> &output) const {
                convert_color<number, rgba>(input, output);
            }

            template<typename CODER2>
            void convert(const color_t &input, color_t &output, const CODER2 &coder2) const {
                // convert input color of my space into a color in coder2 space
                convert_color<rgba, typename CODER2::rgba>(input, output);
            }

            template<typename CODER2>
            void convert(const pixel &input, typename CODER2::pixel &output, const CODER2 &coder2) const {
                color_t input_decoded{};
                decode(input, input_decoded);
                coder2.encode(input_decoded, output);
            }

        };

    }

}
