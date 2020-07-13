#pragma once

#include <ImageWriterWorker.h>
#include <bits.h>
#include <color_t.h>
#include <lodepng.h>
#include <exception>

namespace imagium {

    class png_true_color_worker : public ImageWriterWorker {
    public:
        png_true_color_worker()= default;

        template <typename number=uint32_t >
        number pack_bits_in_number(const color_t & color) {
            ubyte rgba_bits=color.bits_rgba();
            ubyte number_bits= sizeof(number)<<3;
            if(rgba_bits>number_bits)
                throw std::runtime_error{"bad conversion"};
            number storage = (color.r<<color.bits_gba()) +
                             (color.g<<color.bits_ba()) +
                             (color.b<<color.bits_a()) +
                             (color.a<<0);
            return storage;
        }

        std::vector<ubyte> write(byte_array * data, const options & options) const override {
            std::vector<unsigned char> image, image2; //the raw pixels
            unsigned width, height;
            lodepng::State state; //optionally customize this one
            state.info_raw.colortype=LodePNGColorType::LCT_RGB;
            unsigned channels=state.info_raw.colortype==LodePNGColorType::LCT_RGB ? 3:4;
            bits bits_depth=state.info_raw.bitdepth;
            unsigned error = lodepng::decode(image, width, height, state, *data);
            unsigned size=width*height;
            const unsigned pixels = size/channels;
            bits r_bits=options.remap_r, g_bits=options.remap_g, b_bits=options.remap_b;
            for (unsigned ix = 0; ix < pixels; ix+=channels) {
                uint r=convert_channel(image[ix+0], bits_depth, r_bits);
                uint g=convert_channel(image[ix+1], bits_depth, g_bits);
                uint b=convert_channel(image[ix+2], bits_depth, b_bits);
                color_t color{r, g, b, 0, r_bits, g_bits, b_bits, 0};
                infer_power_of_2_bytes_needed_for_rgba(r_bits, g_bits, b_bits, 0);
            }
//            infer_power_of_2_bytes_needed_for_rgba
            convert_channel(1,1,1);
//            state.info_png.color.colortype=LodePNGColorType::LCT_PALETTE;
//            state.encoder.force_palette=1;
//            error = lodepng::encode(image2, image, width, height, state);

            //if there's an error, display it
            if(error) std::cout << "decoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        };
    };
}
