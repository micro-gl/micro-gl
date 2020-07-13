#pragma once

#include <string>
#include <map>
#include <types.h>
#include <bundle.h>
#include <cstdlib>
#include <utility>

namespace imagium {

    class options {
    public:
        image_format input_image_format=image_format::unknown;
        color_format output_color_format=color_format::unknown;
        int r=-1, g=-1, b=-1, a=-1;
        str force_converter="";
        str name="test";
    private:
        bundle _bundle;
    public:
        explicit options(bundle & bundle) : _bundle{std::move(bundle)} {
            r= _bundle.getValueAsInteger("r", 0);
            g= _bundle.getValueAsInteger("g", 0);
            b= _bundle.getValueAsInteger("b", 0);
            a= _bundle.getValueAsInteger("a", 0);
            force_converter= _bundle.getValueAsString("force_converter", "");


        }

        str toString() const {
            if(!force_converter.empty()) return force_converter;
            return image_format_to_string(input_image_format) + "_" +
                                color_format_to_string(output_color_format);
        }
    };
}
