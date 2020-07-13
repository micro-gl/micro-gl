#pragma once

#include <string>
#include <types.h>

namespace imagium {

    struct options {
        options(image_format i, color_format c) : input_image_format{i}, output_color_format{c} {

        }
        image_format input_image_format=image_format::unknown;
        color_format output_color_format=color_format::unknown;
        // todo:: convret all to a map instead
        int remap_r=-1;
        int remap_g=-1;
        int remap_b=-1;
        int remap_a=-1;
        std::string override_worker="";

        std::string toString() const {
            if(!override_worker.empty())
                return override_worker;
            return image_format_to_string(input_image_format) + "_" +
                                color_format_to_string(output_color_format);
        }
    };
}
