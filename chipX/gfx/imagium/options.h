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
        using file_paths=std::vector<str>;
        // common options typed
        image_format input_image_format=image_format::unknown;
        color_format export_format=color_format::unknown;
        int r=-1, g=-1, b=-1, a=-1;
        str converter="";
        str output_name="test";
        str files_path;
    private:
        bundle _bundle;
    public:
        options()= default;
        explicit options(bundle & bundle) : _bundle{std::move(bundle)} {
            r= _bundle.getValueAsInteger("r", 0);
            g= _bundle.getValueAsInteger("g", 0);
            b= _bundle.getValueAsInteger("b", 0);
            a= _bundle.getValueAsInteger("a", 0);
            converter= _bundle.getValueAsString("converter", "");
            output_name= _bundle.getValueAsString("o", "no_name");
            files_path=_bundle.getValueAsString("files", "");
        }

        bundle & extraOptions() { return _bundle; }

        str toString() const {
            if(!converter.empty()) return converter;
            return image_format_to_string(input_image_format) + "_" +
                                color_format_to_string(export_format);
        }
    };
}
