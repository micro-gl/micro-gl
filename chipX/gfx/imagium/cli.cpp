#include <Imagium.h>
#include <iostream>
using namespace imagium;

#define DEBUG 1

const char* s1 =R"foo(usage: imagium [image file] [options]
options include:
  -format                  choose the output format:
                           values are (true_color | palette | grayscale)
                           notes: some formats like palette depend on predefined
                           palette that is embedded in your image

  -rgba                    choose the colors channels bits sizes:
                           values are comma separeted r,g,b,a
                           example: 8,8,8,8 or 5,6,5,0 or 3,0,0,0 etc..

  -converter (optional)    choose a specific converter that you know of. by default,
                           imagium will infer the correct one.
                           example: png_true_color_converter

  -colors                  choose the amount of colors.
                           value can be 2, 4, 16, 256.
                           notes: Only applicable in palette or grayscale formats
)foo";


int main(int argc, char *argv[]) {
    std::cout<<s1<<std::endl;
    auto bundle=bundle::fromTokens(argc, argv);
    auto files= bundle.getValueAsString("VOID_KEY", "");
#if (DEBUG==1)
    files="./assets/uv_256.png";
#endif
    if(files.empty()) {
        std::cerr << "no file specified !!!" << std::endl;
    }

    // obviously a very quick and dirty method
    auto file_type=files.substr(files.find(".")+1);
    auto format=bundle.getValueAsString("format", "unknown");
    auto converter_tag= file_type+"_"+format;
    if(bundle.hasKey("converter")) // force specific converter
        converter_tag=bundle.getValueAsString("converter");
    bundle.putValue("files", files);
    auto * data=imagium::loadFileAsByteArray(files);
    imagium::Imagium lib{};
    imagium::options options{bundle};
    lib.produce(converter_tag, data, options);
}

