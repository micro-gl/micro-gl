#pragma once

#include <string>
#include <map>
#include <cstdlib>
#include <utility>
#include <algorithm>

#include <imagium/Config.h>
#include "bundle.h"
#include "utils.h"

class options {
private:
    using str = std::string;
    using string_list = std::vector<std::string>;
    bundle _bundle;

public:
    imagium::Config config{};
    str files_path="";
public:
    options() {
        config = imagium::Config::getDefault();
    };

    explicit options(bundle & bundle) : _bundle{std::move(bundle)} {

        str default_output_name;
        files_path=_bundle.getValueAsString("VOID_KEY",
                "nada.nada");
        const auto last_of = files_path.find_last_of('.');
        config.image_format = files_path.substr(last_of+1);
        {
            size_t ff= files_path.find_last_of('\\');
            size_t bb= files_path.find_last_of('/');
            ff= (ff==std::string::npos) ? 0 : ff+1;
            bb= (bb==std::string::npos) ? 0 : bb+1;
            const auto ss=std::max({ff,bb,0UL});
            default_output_name = files_path.substr(ss, last_of-ss);
        }
        config.converter= _bundle.getValueAsString("converter", "");
        config.output_name= _bundle.getValueAsString("o", default_output_name);
        config.pack_channels=!_bundle.hasKey("unpack");
        config.use_palette=_bundle.hasKey("indexed");
        auto rgba = _bundle.getValueAsString("rgba", "8-8-8-8");
        string_list delimited= split(rgba, "-");
        delimited.resize(4, "0");
        config.r= delimited[0].empty() ? 0 : std::stoi(delimited[0]);
        config.g= delimited[1].empty() ? 0 : std::stoi(delimited[1]);
        config.b= delimited[2].empty() ? 0 : std::stoi(delimited[2]);
        config.a= delimited[3].empty() ? 0 : std::stoi(delimited[3]);
    }

    bundle & extraOptions() { return _bundle; }
};
