#include <types.h>

namespace imagium {

    std::string image_format_to_string(const image_format &val) {
        std::string res{};
        switch (val) {
            case image_format::PNG :
                res="png"; break;
            case image_format::BMP :
                res="bmp"; break;
            case image_format::JPEG :
                res="jpeg"; break;
            case image_format::unknown :
            default:
                res="unknown"; break;
        }
        return res;
    }

    std::string color_format_to_string(const color_format &val) {
        std::string res{};
        switch (val) {
            case color_format::true_color :
                res="true_color"; break;
            case color_format::true_color_with_alpha :
                res="true_color_with_alpha"; break;
            case color_format::grayscale_2_colors :
                res="grayscale_2_colors"; break;
            case color_format::grayscale_4_colors :
                res="grayscale_2_colors"; break;
            case color_format::grayscale_16_colors :
                res="grayscale_16_colors"; break;
            case color_format::grayscale_256_colors :
                res="grayscale_256_colors"; break;
            case color_format::palette_2_colors :
                res="palette_2_colors"; break;
            case color_format::palette_4_colors :
                res="palette_4_colors"; break;
            case color_format::palette_16_colors :
                res="palette_16_colors"; break;
            case color_format::palette_256_colors :
                res="palette_256_colors"; break;
            default:
                res="unknown"; break;
        }
        return res;
    }
}