#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace imagium {

    using uint= unsigned int;
    using ubyte= unsigned char;
    using channel= uint32_t ;
    using bits= unsigned char;
    using byte_array= std::vector<ubyte>;
    using str= std::string;

    enum class image_format {
        PNG, JPEG, BMP, unknown
    };

    enum class color_format {
        true_color,
        true_color_with_alpha,

        palette_2_colors,
        palette_4_colors,
        palette_16_colors,
        palette_256_colors,

        grayscale_2_colors,
        grayscale_4_colors,
        grayscale_16_colors,
        grayscale_256_colors,

        unknown
    };

    std::string image_format_to_string(const image_format & val);
    std::string color_format_to_string(const color_format & val);
}
