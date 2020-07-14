#pragma once

#include <converter.h>
#include <typed_packed_array.h>
#include <typed_unpacked_array.h>
#include <bits.h>
#include <color_t.h>
#include <utils.h>
#include <lodepng.h>
#include <exception>

namespace imagium {

    class png_true_color_converter : public converter {
    public:
        png_true_color_converter()= default;

        byte_array write(byte_array * data, const options & options) const override {
            std::vector<unsigned char> image, image2; //the raw pixels
            unsigned width, height;
            lodepng::State state; //optionally customize this one
            bits r_bits=options.r, g_bits=options.g, b_bits=options.b, a_bits=options.a;
            // ask for full rgba channels, user can drop channels out easily with options
            state.info_raw.colortype=LodePNGColorType::LCT_RGBA;
            unsigned channels=state.info_raw.colortype==LodePNGColorType::LCT_RGB ? 3:4;
            bits bits_depth=state.info_raw.bitdepth;
            unsigned error = lodepng::decode(image, width, height, state, *data);

            if(error)
                std::cerr << "png_true_color_converter error " << error << ": "<< lodepng_error_text(error) << std::endl;

            const unsigned pixels_count = width*height;

            addressable_ram * array= nullptr;
            if(options.pack_channels)
                array=factory_PackedArray::getArray(width, height, r_bits, g_bits, b_bits, a_bits);
            else
                array=factory_UnpackedArray::getArray(width, height, r_bits, g_bits, b_bits, a_bits);

            for (unsigned ix = 0; ix < pixels_count; ix+=channels) {
                uint r=convert_channel(image[ix+0], bits_depth, r_bits);
                uint g=convert_channel(image[ix+1], bits_depth, g_bits);
                uint b=convert_channel(image[ix+2], bits_depth, b_bits);
                uint a=convert_channel(image[ix+3], bits_depth, a_bits);
                color_t color{r, g, b, a, r_bits, g_bits, b_bits, a_bits};
                array->write(ix, color);
            }

            const str rendered_string= array->toString(options.output_name);
            byte_array result {rendered_string.begin(), rendered_string.end()};
            return result;
            //if there's an error, display it
            if(error) std::cout << "decoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        };
    };
}
