#pragma once

#include "common_types.h"

namespace fontium {

    enum LayoutType {
        box, box_optimal, grid, gridline, line
    };

    struct LayoutConfig {
        static LayoutConfig getDefault() {
            LayoutConfig config;
            config.layout_type=LayoutType::box;
            config.one_pixel_offset=true;
            config.pot_image=false;
            config.size_increment=0;
            config.offset_left=0;
            config.offset_top=0;
            config.offset_right=0;
            config.offset_bottom=0;
            return config;
        }

        static
        LayoutType stringToLayoutTypeEnum(const str & val) {
            if(val=="box")
                return LayoutType::box;
            if(val=="box_optimal")
                return LayoutType::box_optimal;
            if(val=="grid")
                return LayoutType::grid;
            if(val=="gridline")
                return LayoutType::gridline;
            if(val=="line")
                return LayoutType::line;
            return LayoutType::box;
        }

        LayoutType layout_type=LayoutType::box;
        bool one_pixel_offset=true;
        bool pot_image=false;
        int size_increment=0;
        int offset_left=0;
        int offset_top=0;
        int offset_right=0;
        int offset_bottom=0;
    };
}