#include <Fontium.h>
#include <iostream>

//using namespace fontium;
#define DEBUG 0

const char* info =R"foo(usage:
  fontium [font file] [options]

description:
  fontium creates bitmap fonts with custom export formats for
  TrueType, CFF, WOFF, OpenType, SFNT, PCF, FNT, BDF, PFR fonts

options include:
* layout options
  -layout.type                { box, box_optimal, grid, gridline, line }, default=box
  -layout.one_pixel_offset    if set, adds at least one pixel separation between glyphs, default to true
  -layout.pot_image           if set, create power of 2 image, default false
  -layout.offset_left         integer, sets the left padding, default 0
  -layout.offset_top          integer, sets the top padding, default 0
  -layout.offset_right        integer, sets the right padding, default 0
  -layout.offset_bottom       integer, sets the bottom padding, default 0

* font options
  -font.size                  size of font in points, default 14
  -font.dpi                   dots per inch, usually { 72, 96, 100, 110, 120, 128 }, default 72
  -font.characters            (string) the characters, by default will use
                                    " !\"#$%&'()*+,-./0123456789:;<=>?@"
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                                    "abcdefghijklmnopqrstuvwxyz{|}~"
  -font.antialiasing          { None, Normal, Light, LCDH, LCDV }, default Normal
  -font.hinting               { Disable, Default, ForceFreetypeAuto, DisableFreetypeAuto }, default to Default
  -font.scale_width           percentages, scale horizontally every glyph, default is 100
  -font.scale_height          percentages, scale vertically every glyph, default is 100
  -font.char_spacing          integer, add spacing to each glyph advance in export, default 0
  -font.line_spacing          integer, add height to export's gylph metrics baseline, default 0
  -font.bold                  [0, 10] - boldness, , default 0
  -font.italic                [-20, 20] - italicness,  default 0

* output options
  -output.export              { sparrow, c_array }, default to sparrow
  -output.name                name of the export files, default to 'no-name'

* misc
  -h                          show help

example:
  fontium minecraft.ttf -font.size 12 -output.export sparrow -output.name minecraft
)foo";


int main(int argc, char *argv[]) {

    Fontium::Builder builder{};
    Fontium * fontium = builder
        .layoutConfig(nullptr)
        .fontConfig(nullptr)
        .font(nullptr).build();


//    FT_Library  library;
//    int error = FT_Init_FreeType( &library );
//    if ( error )
//    {
//    }
//#if (DEBUG==1)
//    auto bundle_ = bundle{{
//        {"VOID_KEY", "./assets/uv_256.png"},
//        {"rgba", "2|0|0|0"},
////        {"rgba", "5|6|5|0"},
////        {"rgba", "5|2|2"},
//        {"unpack", ""},
////        {"converter", "png_palette_converter"},
//        {"o", "hello"},
//    }};
//#elif (DEBUG==2)
//    // test indexed mode
//    auto bundle_ = bundle{{
//        {"VOID_KEY", "./assets/uv_256_16_colors.png"},
////        {"rgba", "8|8|8|0"},
//        {"rgba", "8|0|0|0"},
//        {"indexed", ""},
////        {"rgba", "5|6|5|0"},
//        {"unpack", ""},
//        }};
//#else
//    auto bundle_=bundle::fromTokens(argc, argv);
//#endif
//    if(bundle_.hasKey("h") || bundle_.size()<=1) {
//        std::cout << info << std::endl;
//        return 0;
//    }
//    if(bundle_.getValueAsString("VOID_KEY", "").empty()) {
//        std::cout << "Error: no file specified !!!" << std::endl;
//        return 0;
//    }
//
//    try {
//        std::cout << "Imagium CLI" << std::endl;
//        fontium::Fontium lib{};
//        fontium::options options{bundle_};
//        auto * data=fontium::loadFileAsByteArray(options.files_path);
////        auto result = lib.produce(data, options);
////        str test(reinterpret_cast<char *>(result.data.data()), result.data.size());
////        std::ofstream out(options.output_name + ".h");
////        out << test;
////        out.close();
////        std::cout << "created :: " << options.output_name + ".h, " << result.size_bytes/1024 << "kb" <<std::endl;
//    }
//    catch (const std::exception& e){
//        std::cout << "Imagium error: " + str{e.what()} << std::endl;
//        return 1;
//    }

}

