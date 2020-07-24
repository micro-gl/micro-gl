#include <lodepng.h>
#include <iostream>
#include <fontium/Fontium.h>
#include <fontium/ExportFactory.h>
#include "cli/options.h"
#include "cli/utils.h"

using namespace fontium;
#define DEBUG 1

const char* info =R"foo(usage:
  fontium <font path> [options]

description:
  fontium creates bitmap fonts with custom export formats for
  TrueType, CFF, WOFF, OpenType, SFNT, PCF, FNT, BDF, PFR fonts

options include:
* FONT options
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
  -font.face_index            the face index to load,  default 0

* LAYOUT options
  -layout.type                { box, box_optimal, grid, gridline, line }, default=box
  -layout.one_pixel_offset    ( false | true) if set, adds at least one pixel separation between glyphs, default to true
  -layout.pot_image           ( false | true) if set, create power of 2 image, default false
  -layout.offset_left         integer, sets the left padding, default 0
  -layout.offset_top          integer, sets the top padding, default 0
  -layout.offset_right        integer, sets the right padding, default 0
  -layout.offset_bottom       integer, sets the bottom padding, default 0

* OUTPUT options
  -output.export              { bmf (Angel Code), c_array }, default to sparrow
  -output.name                name of the export files, default to <font-name>

* misc
  -h                          show help

example:
  fontium minecraft.ttf -font.size 12 -output.export bmf -output.name minecraft

Author:
  Tomer Shalev (tomer.shalev@gmail.com)
)foo";


int main(int argc, char *argv[]) {

#if (DEBUG==1)
    // test true type fonts
    auto bundle_ = bundle{{
        {"VOID_KEY", "./assets/digital-7.ttf"},
        {"font.size", "48"},
//        {"font.antialiasing", "None"},
//        {"font.hinting", "Disabled"},
        {"layout.one_pixel_offset", "true"},
        {"layout.type", "box"},
//        {"layout.type", "line"},
        {"output.export", "bmf"},
        {"output.name", "tomer211"},
//        {"h", ""},
    }};
#elif (DEBUG==2)
    // test bdf / pcf fonts
    auto bundle_ = bundle{{
//          {"VOID_KEY", "./assets/bitbuntu-full.bdf"},
//          {"VOID_KEY", "./assets/creep.bdf"},
//          {"VOID_KEY", "./assets/creep2-16.bdf"},
          {"VOID_KEY", "./assets/TerminusBold-16.bdf"},
//          {"VOID_KEY", "./assets/dweep.pcf"},
          {"font.size", "48"},
          {"font.antialiasing", "None"},
          {"layout.type", "box"},
          {"output.export", "bmf"},
    }};
#else
    auto bundle_=bundle::fromTokens(argc, argv);
#endif
    const bool show_help = bundle_.hasKey("h") ||
            bundle_.getValueAsString("VOID_KEY", "").empty();
    if(show_help) {
        std::cout << info << std::endl;
        return 0;
    }

    try {
        std::cout << "Fontium CLI" << std::endl;

        // extract bundle into options
        fontium::options options{bundle_};
        auto * font=loadFileAsByteArray(options.input_font_path);
        str basename= options.output_export_name;
        str image_file_name= basename + ".png";

        auto bm_font = Fontium::create(
                basename,
                *font,
                options.fontConfig,
                options.layoutConfig);

        bm_font.image_file_name= image_file_name;
        auto * exporter = ExportFactory::create(options.output_export_type);
        str result = exporter->apply(bm_font);
        str data_file_name= basename + "." + exporter->fileExtension();

        delete exporter;
        // write data file
        std::ofstream out(data_file_name);
        out << result;
        out.close();
        std::cout << std::endl <<  "    created data file :: " << data_file_name
                                        << std::endl;

        // png
        lodepng::State state;
        // input color type
        state.info_raw.colortype = LCT_GREY;
        state.info_raw.bitdepth = 8;
        // output color type
        state.info_png.color.colortype = LCT_GREY;
        state.info_png.color.bitdepth = 8;
        // without this, it would ignore the output color type specified
        // above and choose an optimal one instead
        state.encoder.auto_convert = 0;

        //encode and save
        std::vector<unsigned char> buffer;
        unsigned error = lodepng::encode(buffer, bm_font.img->data(),
                bm_font.img->width(),
                bm_font.img->height(),
                state);
        if(error) {
            throw std::runtime_error("encoder error " + std::to_string(error) + ": "
            + lodepng_error_text(error) );
        }
        else {
            lodepng::save_file(buffer, image_file_name);
            std::cout << "    created image file :: " << image_file_name <<std::endl;
        }

    }
    catch (const std::exception& e){
        std::cout << std::endl << "    Fontium error:" << std::endl << "    - "
                                    << str{e.what()} << std::endl;
        return 1;
    }

    return 0;
}

