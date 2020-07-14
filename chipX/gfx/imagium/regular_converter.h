#pragma once

#include "libs/stb_image/stb_image.h"
#include <converter.h>
#include <typed_packed_array.h>
#include <typed_unpacked_array.h>
#include <bits.h>
#include <color_t.h>
#include <utils.h>
#include <lodepng.h>
#include <exception>

namespace imagium {

    class regular_converter : public converter {
    public:
        regular_converter()= default;

        byte_array write(byte_array * data, const options & options) const override {
            int width, height, input_channels=4, bits_depth=8;
            bits r_bits=options.r, g_bits=options.g, b_bits=options.b, a_bits=options.a;

            /*
            std::vector<unsigned char> image; //the raw pixels
            lodepng::State state; //optionally customize this one
            state.info_raw.colortype=LodePNGColorType::LCT_RGBA;
            input_channels=state.info_raw.colortype==LodePNGColorType::LCT_RGB ? 3:4;
            bits_depth=state.info_raw.bitdepth;
            unsigned error = lodepng::decode(image, width, height, state, *data);
            if(error) std::cerr << "regular_converter error " << error << ": "<< lodepng_error_text(error) << std::endl;
             */

            ubyte * image=nullptr;
            {
                int nrChannels;
                image=stbi_load_from_memory(data->data(), data->size(), &width, &height, &nrChannels, input_channels);
            }

            const unsigned pixels_count = width*height;

            addressable_ram * array= nullptr;
            if(options.pack_channels)
                array=factory_PackedArray::getArray(width, height, r_bits, g_bits, b_bits, a_bits);
            else
                array=factory_UnpackedArray::getArray(width, height, r_bits, g_bits, b_bits, a_bits);

            for (unsigned ix = 0, jx=0; ix < pixels_count; ++ix, jx+=input_channels) {
                uint r=convert_channel(image[jx+0], bits_depth, r_bits);
                uint g=convert_channel(image[jx+1], bits_depth, g_bits);
                uint b=convert_channel(image[jx+2], bits_depth, b_bits);
                uint a=convert_channel(image[jx+3], bits_depth, a_bits);
                color_t color{r, g, b, a, r_bits, g_bits, b_bits, a_bits};
                array->write(ix, color);
            }

            const str rendered_string= array->toString(options.output_name);
            byte_array result {rendered_string.begin(), rendered_string.end()};
            return result;
        };
    };
}
