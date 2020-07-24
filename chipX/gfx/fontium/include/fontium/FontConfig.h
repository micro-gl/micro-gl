#pragma once

#include "common_types.h"

namespace fontium {

    enum Hinting {
        Disable=0,
        Default=1,
        ForceFreetypeAuto=2,
        DisableFreetypeAuto=3
    };

    enum Antialiasing {
        None=0,
        Normal=1,
        Light=2,
        LCDH=3,
        LCDV=4
    };

    struct FontConfig {
        static FontConfig getDefault() {
            FontConfig config;
            config.size = 16;
            config.hinting = Hinting::Default;
            config.render_missing = false;
            config.antialiasing = Antialiasing::Normal;
            config.bold = 0;
            config.scale_width = 100.0f;
            config.scale_height = 100.0f;
            config.line_spacing = 0;
            config.char_spacing = 0;
            config.dpi = 72;
            config.characters = " !\"#$%&'()*+,-./0123456789:;<=>?@"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                                "abcdefghijklmnopqrstuvwxyz{|}~";
            return config;
        }

        static
        Hinting stringToHintingEnum(const str & val) {
            if(val=="Disable")
                return Hinting::Disable;
            if(val=="Default")
                return Hinting::Default;
            if(val=="ForceFreetypeAuto")
                return Hinting::ForceFreetypeAuto;
            if(val=="DisableFreetypeAuto")
                return Hinting::DisableFreetypeAuto;
            return Hinting::Default;
        }

        static
        Antialiasing stringToAAEnum(const str & val) {
            if(val=="Normal")
                return Antialiasing::Normal;
            if(val=="None")
                return Antialiasing::None;
            if(val=="Light")
                return Antialiasing::Light;
            if(val=="LCDH")
                return Antialiasing::LCDH;
            if(val=="LCDV")
                return Antialiasing::LCDV;
            return Antialiasing::Normal;
        }

//        str path;
//        str filename;
        str characters;
        int face_index;
        int size;
        Hinting hinting;
        bool render_missing;
        Antialiasing antialiasing;
        int bold;
        int italic;
        float scale_width;
        float scale_height;
        int char_spacing;
        int line_spacing;
        int dpi;
    };
}