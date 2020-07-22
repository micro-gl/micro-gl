#pragma once

#include "common_types.h"

namespace fontium {

    struct LayoutConfig {
        str type;
        bool one_pixel_offset;
        bool pot_image;
        int size_increment;
        int offset_left;
        int offset_top;
        int offset_right;
        int offset_bottom;
    };
}