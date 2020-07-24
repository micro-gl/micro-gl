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
        palette,
        grayscale,
        unknown
    };

    std::string image_format_to_string(const image_format & val);
    std::string color_format_to_string(const color_format & val);
    image_format string_to_image_format(const str &val);
    color_format string_to_color_format(const str &val);
}
