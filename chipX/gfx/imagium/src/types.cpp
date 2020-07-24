#include "types.h"

namespace imagium {

    std::string image_format_to_string(const image_format &val) {
        std::string res{};
        switch (val) {
            case image_format::PNG :
                res="png"; break;
            case image_format::BMP :
                res="bmp"; break;
            case image_format::JPEG :
                res="jpeg"; break;
            case image_format::unknown :
            default:
                res="unknown"; break;
        }
        return res;
    }

    std::string color_format_to_string(const color_format &val) {
        std::string res{};
        switch (val) {
            case color_format::true_color :
                res="true_color"; break;
            case color_format::palette :
                res="palette"; break;
            case color_format::grayscale :
                res="grayscale"; break;
            default:
                res="unknown"; break;
        }
        return res;
    }

    image_format string_to_image_format(const str &val) {
        if(val=="PNG") return image_format::PNG;
        else if(val=="JPEG") return image_format::JPEG;
        else if(val=="BMP") return image_format::BMP;
        return image_format::unknown;
    }

    color_format string_to_color_format(const str &val) {
        if(val=="true_color") return color_format::true_color;
        else if(val=="palette") return color_format::palette;
        else if(val=="grayscale") return color_format::grayscale;
        return color_format::unknown;
    }
}