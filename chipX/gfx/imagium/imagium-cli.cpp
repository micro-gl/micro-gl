#include <Imagium.h>
#include <iostream>
using namespace imagium;

#define DEBUG 0

const char* info =R"foo(usage:
  imagium [image file] [options]

description:
  imagium reshapes images and exports them into software arrays, so you can
  embed them in your program in the pixel format of your liking

options include:
  -rgba         value: R-G-B-A (literally)
                values are bits per channel
                example: 8-8-8-8 or 5-6-5-0 or 3-0-0-0 etc..
                note: 0 bits for a channel will completely discard the channel
  -unpack       if set, unpacks each channel separately inside a power of 2 number type = {r,g,b,a, r,g,b,a ...}
                if not set (default), packs all channels inside a power of 2 number type = {pix1, pix2, ...}
                notes: defaults to `true`, `false`, is desirable for 8|8|8|0 rgb config, where
                you will get the buffer = {r,g,b, r,g,b, .....} which is more optimal than packed.
  -indexed      create indexed data with embedded palette of the image
  -o            (optional) output name, if not set, will try to use file name
  -converter    (optional) value: converter-name
                choose a specific converter that you know of. by default,
                imagium will infer the correct one.
                example: regular_converter
  -h            show help

example:
  imagium foo.png -rgba 5-6-5
  imagium foo_with_16_color_palette.png -rgba 5-6-5 -indexed
)foo";


int main(int argc, char *argv[]) {
#if (DEBUG==1)
    auto bundle_ = bundle{{
        {"VOID_KEY", "./assets/uv_256.png"},
        {"rgba", "2|0|0|0"},
//        {"rgba", "5|6|5|0"},
//        {"rgba", "5|2|2"},
        {"unpack", ""},
//        {"converter", "png_palette_converter"},
        {"o", "hello"},
    }};
#elif (DEBUG==2)
    // test indexed mode
    auto bundle_ = bundle{{
        {"VOID_KEY", "./assets/uv_256_16_colors.png"},
//        {"rgba", "8|8|8|0"},
        {"rgba", "8|0|0|0"},
        {"indexed", ""},
//        {"rgba", "5|6|5|0"},
        {"unpack", ""},
        }};
#else
    auto bundle_=bundle::fromTokens(argc, argv);
#endif
    if(bundle_.hasKey("h") || bundle_.size()<=1) {
        std::cout << info << std::endl;
        return 0;
    }
    if(bundle_.getValueAsString("VOID_KEY", "").empty()) {
        std::cout << "Error: no file specified !!!" << std::endl;
        return 0;
    }

    try {
        std::cout << "Imagium CLI" << std::endl;
        imagium::Imagium lib{};
        imagium::options options{bundle_};
        auto * data=imagium::loadFileAsByteArray(options.files_path);
        auto result = lib.produce(data, options);
        str test(reinterpret_cast<char *>(result.data.data()), result.data.size());
        std::ofstream out(options.output_name + ".h");
        out << test;
        out.close();
        std::cout << "created :: " << options.output_name + ".h, " << result.size_bytes/1024 << "kb" <<std::endl;
    }
    catch (const std::exception& e){
        std::cout << "Imagium error: " + str{e.what()} << std::endl;
        return 1;
    }

}

