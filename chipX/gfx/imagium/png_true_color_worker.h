#pragma once

#include <ImageWriterWorker.h>
#include <bits.h>
#include <color_t.h>
#include <utils.h>
#include <lodepng.h>
#include <exception>

namespace imagium {

    class addressable_ram {
    public:
        virtual void write(unsigned index, uint64_t value) = 0;
        virtual void write(unsigned index, const color_t &color) = 0;
        virtual uint64_t operator[](int index) const = 0;
        virtual str toString(const str &name) const = 0;
    };

    template <typename number>
    class typed_packed_array : public addressable_ram {
        std::vector<number> _data;
        const uint _w, _h;
        const bits _r_bits, _g_bits, _b_bits, _a_bits;
        unsigned _pixel_storage_size_bits=0; // power of 2
        unsigned _pixel_size_bits=0; // <= _pixel_storage_size_bits
        unsigned _pixels_count=0;
        const unsigned buffer_element_size_bits= sizeof(number)*8;
        number _mask;
    public:
            explicit typed_packed_array(uint w, uint h,
                    bits r_bits, bits g_bits, bits b_bits,bits a_bits) : _w{w}, _h{h},
                                _r_bits{r_bits}, _g_bits{g_bits}, _b_bits{b_bits}, _a_bits{a_bits} {
            _mask=(number(1)<<_pixel_storage_size_bits)-1;
            _pixels_count=_w*_h;
            _pixel_size_bits=r_bits+g_bits+b_bits+a_bits;
            _pixel_storage_size_bits=infer_power_of_2_bits_needed_from_bits(_pixel_size_bits);
            uint size= (_pixel_storage_size_bits*_pixels_count)/ buffer_element_size_bits;
            _data.resize(size);
        }

        void write(unsigned index, const color_t &color) override {
            const number packed=pack_bits_in_number(color);
            write(index, packed);
        }

        void write(unsigned index, uint64_t value) override {
            // mask the value to the lower pixel storage bits window
            number value_masked=number(value)&_mask;
            // index of containing element inside the elements array
            unsigned int idx2=(index*_pixel_storage_size_bits)/buffer_element_size_bits;
            // distance in bits from beginning of storage window to beginning of block
            unsigned int D=(index*_pixel_storage_size_bits) - idx2*buffer_element_size_bits;
            // get the element to update
            number element_to_change=_data[idx2];
            // create a mask to extract zero the place where the pixel should lay
            // by moving the mask to the correct place
            const number clear_mask= (_mask) << (D);
            // clear the bits in the designated pixel position
            element_to_change &= (~clear_mask);
            // move the new value int place
            number new_element= (value_masked) << (D);
            // blend the element. the value was masked in the beginning with zeroes, so all good
            new_element = element_to_change | new_element;
            // write the modified element
            _data[idx2]=new_element;
        }

        uint64_t operator[](int index) const override {
            return _data[index];
        }

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

        str toString(const str &name) const override {
            const str type_name=infer_type_needed_for_bits(buffer_element_size_bits);
            const str space=" ";
            str var= type_name + space + name + "[" + std::to_string(_w) + "*" + std::to_string(_h) +"]= {";
            const auto buffer_size=_data.size();
            for (unsigned long ix = 0; ix < buffer_size; ++ix) {
                const auto hex_str=intToHexString<number>(_data[ix]);
                var+=hex_str;
                if(ix<buffer_size-1) { // not last one
                    var += ", ";
                }
            }
            var+="}";
            return var;
        }
    };

    class factory_PackedArray {
    public:
        factory_PackedArray()= delete;

        static
        addressable_ram * getPackedArray(uint w, uint h,
                                         bits r_bits, bits g_bits, bits b_bits, bits a_bits) {
            ubyte needed_power_of_2_pixel_bits=infer_power_of_2_bits_needed_from_bits(r_bits+g_bits+b_bits+a_bits);
            const unsigned bytes_per_element=infer_power_of_2_bytes_needed_from_bits(needed_power_of_2_pixel_bits);
            switch (bytes_per_element) {
                case 1: return new typed_packed_array<uint8_t>{w, h, r_bits, g_bits, b_bits, a_bits};
                case 2: return new typed_packed_array<uint16_t>{w, h, r_bits, g_bits, b_bits, a_bits};
                case 4: return new typed_packed_array<uint32_t>{w, h, r_bits, g_bits, b_bits, a_bits};
                case 8: return new typed_packed_array<uint64_t>{w, h, r_bits, g_bits, b_bits, a_bits};
                default: return new typed_packed_array<uint64_t>{w, h, r_bits, g_bits, b_bits, a_bits};
            }
        }

    };


    class png_true_color_worker : public ImageWriterWorker {
    public:
        png_true_color_worker()= default;

        std::vector<ubyte> write(byte_array * data, const options & options) const override {
            std::vector<unsigned char> image, image2; //the raw pixels
            unsigned width, height;
            lodepng::State state; //optionally customize this one
            state.info_raw.colortype=LodePNGColorType::LCT_RGB;
            unsigned channels=state.info_raw.colortype==LodePNGColorType::LCT_RGB ? 3:4;
            bits bits_depth=state.info_raw.bitdepth;
            unsigned error = lodepng::decode(image, width, height, state, *data);
            const unsigned pixels_count = width*height;
            bits r_bits=options.r, g_bits=options.g, b_bits=options.b, a_bits=options.a;
            addressable_ram * array= factory_PackedArray::getPackedArray(width, height, r_bits, g_bits, b_bits, a_bits);
            for (unsigned ix = 0; ix < pixels_count; ix+=channels) {
                uint r=convert_channel(image[ix+0], bits_depth, r_bits);
                uint g=convert_channel(image[ix+1], bits_depth, g_bits);
                uint b=convert_channel(image[ix+2], bits_depth, b_bits);
                color_t color{r, g, b, 0, r_bits, g_bits, b_bits, 0};
                array->write(ix, color);
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
