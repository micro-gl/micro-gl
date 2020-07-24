#pragma once

#include <string>
#include <map>
#include <cstdlib>
#include <utility>

namespace imagium {

    struct Config {

        using str=std::string;
        using strings=std::vector<str>;

        static
        Config getDefault() {
            Config config{};

            config.image_format="png";
            config.r=8;
            config.g=8;
            config.b=8;
            config.a=8;
            config.pack_channels=true;
            config.use_palette=false;
            config.converter="";
            config.output_name="test";

            return config;
        };

        str image_format;
        int r=-1, g=-1, b=-1, a=-1;
        bool pack_channels=true;
        bool use_palette=false;
        str converter="";
        str output_name="test";
    };
}
