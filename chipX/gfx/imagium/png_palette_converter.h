#pragma once

//#include "libs/stb_image/stb_image.h"
#include "libs/lodepng/lodepng.h"
#include <converter.h>
#include <typed_packed_array.h>
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
            uint width, height, input_channels=4, bits_depth=8, bits_output;
            bits r_bits=options.r, g_bits=options.g, b_bits=options.b, a_bits=options.a;
            uint out_channels= (r_bits?1:0)+(g_bits?1:0)+(b_bits?1:0)+(a_bits?1:0);

            std::vector<unsigned char> image; //the raw pixels
            lodepng::State state; //optionally customize this one
            // request a palette on the output
            state.info_raw.colortype=LodePNGColorType::LCT_PALETTE;
            unsigned error = lodepng::decode(image, width, height, state, *data);
            bits_depth=state.info_png.color.bitdepth;
            ubyte * palette=state.info_png.color.palette;
            ubyte palette_number_of_pixels=state.info_png.color.palettesize;
            if(error)
                throw std::runtime_error("png_palette_converter error "+ std::to_string(error)+
                ": "+ lodepng_error_text(error));

            const unsigned pixels_count = width*height;
            addressable_ram * index_array= nullptr, *palette_array=nullptr;

            { // palette
                auto bits_needed= r_bits+g_bits+b_bits+a_bits;
                uint how_many= palette_number_of_pixels;
                if(!options.pack_channels) {
                    bits_needed=std::max<bits>({r_bits, g_bits, b_bits, a_bits});
                    how_many=width*height*out_channels;
                }
                bits_needed=infer_power_of_2_bits_needed_from_bits(bits_needed);
                palette_array=factory_PackedArray::getArray(how_many, bits_needed);

                for (unsigned ix = 0, jx=0; ix < palette_number_of_pixels; ++ix, jx+=input_channels) {
                    uint r=convert_channel(palette[jx+0], bits_depth, r_bits);
                    uint g=convert_channel(palette[jx+1], bits_depth, g_bits);
                    uint b=convert_channel(palette[jx+2], bits_depth, b_bits);
                    uint a=convert_channel(palette[jx+3], bits_depth, a_bits);
                    color_t color{r, g, b, a, r_bits, g_bits, b_bits, a_bits};
                    if(options.pack_channels) {
                        uint64_t val=pack_bits_in_number(color);
                        palette_array->write(ix, val);
                    } else {
                        unsigned idx2=ix*out_channels;
                        unsigned acc=0;
                        if(r_bits!=0) palette_array->write(idx2+acc++, color.r);
                        if(g_bits!=0) palette_array->write(idx2+acc++, color.g);
                        if(b_bits!=0) palette_array->write(idx2+acc++, color.b);
                        if(a_bits!=0) palette_array->write(idx2+acc++, color.a);
                    }
                }
            }
            { // index map
                unsigned bpp=infer_bits_used_from_unsigned_byte_value(palette_number_of_pixels-1);
                bpp=infer_power_of_2_bits_needed_from_bits(bpp);
                index_array=factory_PackedArray::getArray(width*height, bpp);
                for (unsigned ix = 0; ix < pixels_count; ++ix) {
                    // lode png serves unpacked values for indices, so we can pack them
                    index_array->write(ix, image[ix]);
                }
            }
            str all;
            { // generate strings

                const str palette_string= palette_array->toString(options.output_name + "_palette");
                const str palette_comment= generate_comment(palette_number_of_pixels, 1,
                        palette_array->bit_per_element(), palette_array->bit_per_storage(),
                        r_bits, g_bits, b_bits, a_bits, options.pack_channels);
                const str indexed_string= index_array->toString(options.output_name);
                const str index_comment= generate_comment(width, height,
                        index_array->bit_per_element(), index_array->bit_per_storage(),
                        index_array->bit_per_element(), 0, 0, 0, true);
                all=palette_comment + "\n" + palette_string + "\n\n" +
                        index_comment + "\n" + indexed_string + "\n";

            }
            byte_array result{all.begin(), all.end()};
            return result;
        };

        static
        str generate_comment(int w, int h, bits element_bits, bits storage_bits,
                bits r, bits g, bits b, bits a, bool isPacked) {
            str result;
            char arr[250];
            if(isPacked) {
                uint elements_per_storage=storage_bits/element_bits;
                sprintf(arr, "// %dx%d pixels in this array. each pixel is bit packed (%d|%d|%d|%d) "
                             "inside %d bits block, \n// laid in a %d bits of array storage. "
                             "Each array element contains %d pixels", w,h,r,g,b,a, element_bits,
                             storage_bits, elements_per_storage);
            } else {
                sprintf(arr, "// %dx%d pixels in this array. each channel of pixel is un-packed "
                             "(%d|%d|%d|%d) inside %d bits block of array storage. "
                             "\n// [r,g,b,a, r,g,b,a , .....], note 0 bit channels are removed",
                             w,h, r,g,b,a, storage_bits);
            }
            result=str{arr};
            return result;
        }

        uint64_t pack_bits_in_number(const color_t &color) const {
            uint64_t storage = (color.r << color.bits_gba()) +
                               (color.g << color.bits_ba()) +
                               (color.b << color.bits_a()) +
                               (color.a << 0);
            return storage;
        }

    };
}
