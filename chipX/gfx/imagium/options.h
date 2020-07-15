#pragma once

#include <string>
#include <map>
#include <types.h>
#include <bundle.h>
#include <cstdlib>
#include <utility>
#include <utils.h>
#include <algorithm>

namespace imagium {

    class options {
    public:
        using strings=std::vector<str>;
        str image_format;
        int r=-1, g=-1, b=-1, a=-1;
        bool pack_channels=true;
        bool use_palette=false;
        str converter="";
        str output_name="test";
        str files_path;
    private:
        bundle _bundle;
    public:
        options()= default;
        explicit options(bundle & bundle) : _bundle{std::move(bundle)} {
            str default_output_name;
            files_path=_bundle.getValueAsString("VOID_KEY", "nada.nada");
            const auto last_of = files_path.find_last_of('.');
            image_format = files_path.substr(last_of+1);
            {
                size_t ff= files_path.find_last_of('\\');
                size_t bb= files_path.find_last_of('/');
                ff= (ff==std::string::npos) ? 0 : ff+1;
                bb= (bb==std::string::npos) ? 0 : bb+1;
                const auto ss=std::max({ff,bb,0UL});
                default_output_name = files_path.substr(ss, last_of-ss);
            }
            converter= _bundle.getValueAsString("converter", "");
            output_name= _bundle.getValueAsString("o", default_output_name);
            pack_channels=!_bundle.hasKey("unpack");
            use_palette=_bundle.hasKey("indexed");
            auto rgba = _bundle.getValueAsString("rgba", "8|8|8|8");
            strings delimited= split(rgba, "-");
            delimited.resize(4, "0");
            r= delimited[0].empty() ? 0 : std::stoi(delimited[0]);
            g= delimited[1].empty() ? 0 : std::stoi(delimited[1]);
            b= delimited[2].empty() ? 0 : std::stoi(delimited[2]);
            a= delimited[3].empty() ? 0 : std::stoi(delimited[3]);
        }

        bundle & extraOptions() { return _bundle; }
    };
}
