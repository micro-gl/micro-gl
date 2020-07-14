#pragma once

#include <string>
#include <map>
#include <types.h>
#include <bundle.h>
#include <cstdlib>
#include <utility>
#include <utils.h>

namespace imagium {

    class options {
    public:
        using strings=std::vector<str>;
        // common options typed
        image_format input_image_format=image_format::unknown;
        color_format export_format=color_format::unknown;
        int r=-1, g=-1, b=-1, a=-1;
        bool pack_channels=true;
        str converter="";
        str output_name="test";
        str files_path;
    private:
        bundle _bundle;
    public:
        options()= default;
        explicit options(bundle & bundle) : _bundle{std::move(bundle)} {
            converter= _bundle.getValueAsString("converter", "");
            output_name= _bundle.getValueAsString("o", "no_name");
            files_path=_bundle.getValueAsString("files", "");
            pack_channels=_bundle.getValueAsBoolean("files",  true);
            auto rgba = _bundle.getValueAsString("rgba", "8|8|8|8");
//            strings delimited{4, "0"};
            strings delimited= split(rgba, "|");
            delimited.resize(4, "0");
            r= delimited[0].empty() ? 0 : std::stoi(delimited[0]);
            g= delimited[1].empty() ? 0 : std::stoi(delimited[1]);
            b= delimited[2].empty() ? 0 : std::stoi(delimited[2]);
            a= delimited[3].empty() ? 0 : std::stoi(delimited[3]);
        }

        bundle & extraOptions() { return _bundle; }

        str toString() const {
            if(!converter.empty()) return converter;
            return image_format_to_string(input_image_format) + "_" +
                                color_format_to_string(export_format);
        }
    };
}
