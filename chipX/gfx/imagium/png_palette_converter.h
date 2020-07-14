#pragma once

//#include "libs/stb_image/stb_image.h"
#include "libs/lodepng/lodepng.h"
#include <converter.h>
#include <typed_packed_array.h>
#include <typed_unpacked_array.h>
#include <bits.h>
#include <color_t.h>
#include <utils.h>
#include <lodepng.h>
#include <exception>

namespace imagium {

    class png_palette_converter : public converter {
    public:
        png_palette_converter()= default;

        byte_array write(byte_array * data, const options & options) const override {
            uint width, height, input_channels=4, bits_depth=8, palette_size;
            bits r_bits=options.r, g_bits=options.g, b_bits=options.b, a_bits=options.a;

            std::vector<unsigned char> image; //the raw pixels
            lodepng::State state; //optionally customize this one
            // request a palette on the output
            state.info_raw.colortype=LodePNGColorType::LCT_PALETTE;
            unsigned error = lodepng::decode(image, width, height, state, *data);
            bits_depth=state.info_png.color.bitdepth;
            ubyte * palette=state.info_png.color.palette;
            palette_size=state.info_png.color.palettesize;
            if(error) std::cerr << "png_palette_converter error " << error << ": "<< lodepng_error_text(error) << std::endl;

            const unsigned pixels_count = width*height;
            addressable_ram * array= nullptr, *palette_array=nullptr;

            { // palette
                if(options.pack_channels)
                    palette_array=factory_PackedArray::getArray(palette_size, r_bits, g_bits, b_bits, a_bits);
                else
                    palette_array=factory_UnpackedArray::getArray(palette_size, r_bits, g_bits, b_bits, a_bits);

                for (unsigned ix = 0, jx=0; ix < palette_size; ++ix, jx+=4) {
                    uint r=convert_channel(palette[jx+0], bits_depth, r_bits);
                    uint g=convert_channel(palette[jx+1], bits_depth, g_bits);
                    uint b=convert_channel(palette[jx+2], bits_depth, b_bits);
                    uint a=convert_channel(palette[jx+3], bits_depth, a_bits);
                    color_t color{r, g, b, a, r_bits, g_bits, b_bits, a_bits};
                    palette_array->write(ix, color);
                }
            }
            { // index map
                palette_array=factory_PackedArray::getArray(palette_size, r_bits, g_bits, b_bits, a_bits);
                for (unsigned ix = 0; ix < pixels_count; ++ix) {
//                    uint r=convert_channel(image[jx+0], bits_depth, r_bits);
//                    uint g=convert_channel(image[jx+1], bits_depth, g_bits);
//                    uint b=convert_channel(image[jx+2], bits_depth, b_bits);
//                    uint a=convert_channel(image[jx+3], bits_depth, a_bits);
//                    color_t color{r, g, b, a, r_bits, g_bits, b_bits, a_bits};

                    array->write(ix, color);
                }
            }

            const str rendered_string= array->toString(options.output_name);
            byte_array result {rendered_string.begin(), rendered_string.end()};
            return result;
        };
    };
}
