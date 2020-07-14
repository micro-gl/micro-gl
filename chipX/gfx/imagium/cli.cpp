#include <Imagium.h>
#include <iostream>
using namespace imagium;

#define DEBUG 1

const char* info =R"foo(usage:
  imagium [image file] [options]

description:
  imagium reshapes images and exports them into software arrays, so you can
  embed them in your program in the pixel format of your liking

options include:
  -format                  value: ( true_color | palette | grayscale )
                           notes: some formats like palette depend on predefined
                           palette that is embedded in your image

  -rgba                    value: `R|G|B|A` (literally)
                           values are bits per channel
                           example: 8|8|8|8 or 5|6|5|0 or 3|0|0|0 etc..
                           note: 0 bits for a channel will completely discard the channel

  -colors                  value: ( 2 | 4 | 16 | 256 )
                           amount of colors in palette or grayscale image
                           notes: Only applicable in palette or grayscale formats

  -pack                    value: ( false | true )
                           if true, packs pixel inside a power of 2 number type = {pix1, pix2, ...}
                           if false, packs each channel separately inside a power of 2 number type = {r,g,b,a, r,g,b,a ...}

                           notes: defaults to `true`, `false`, is desirable for 8|8|8|0 rgb config, where
                           you will get the buffer = {r,g,b, r,g,b, .....} which is more optimal than packed.

  -converter (optional)    value: converter-name
                           choose a specific converter that you know of. by default,
                           imagium will infer the correct one.
                           example: png_true_color_converter

)foo";


int main(int argc, char *argv[]) {
    std::cout<< info <<std::endl;
#if (DEBUG==1)
    auto bundle_ = bundle{{{
        {"VOID_KEY", "./assets/uv_256.png"},
        {"format", "true_color"},
        {"rgba", "8|8|8|0"},
        {"pack", "false"},
//        {"rgba", "5|6|5|0"},
    }}};
#elif
    auto bundle=bundle::fromTokens(argc, argv);
#endif
    auto files= bundle_.getValueAsString("VOID_KEY", "");
    if(files.empty()) {
        std::cerr << "no file specified !!!" << std::endl;
    }

    // obviously a very quick and dirty method
    auto file_type=files.substr(files.size()-3);
    auto format=bundle_.getValueAsString("format", "unknown");
    auto converter_tag= file_type+"_"+format;
    if(bundle_.hasKey("converter")) // force specific converter
        converter_tag=bundle_.getValueAsString("converter");
    bundle_.putValue("files", files);
    auto * data=imagium::loadFileAsByteArray(files);
    imagium::Imagium lib{};
    imagium::options options{bundle_};
    byte_array result= lib.produce(converter_tag, data, options);
    int a=0;
}

