#pragma once

#include <addressable_ram.h>
#include <bits.h>
#include <utils.h>
#include <exception>

namespace imagium {
    
    template<typename number>
    class typed_packed_array : public addressable_ram {
        std::vector<number> _data;
        const uint _w, _h;
        const bits _r_bits, _g_bits, _b_bits, _a_bits;
        unsigned _pixel_storage_size_bits = 0; // power of 2
        unsigned _pixel_size_bits = 0; // <= _pixel_storage_size_bits
        unsigned _pixels_count = 0;
        const unsigned buffer_element_size_bits = sizeof(number) * 8;
        number _mask;
    public:
        explicit typed_packed_array(uint w, uint h,
                                    bits r_bits, bits g_bits, bits b_bits, bits a_bits) : _w{w}, _h{h},
                                                                                          _r_bits{r_bits},
                                                                                          _g_bits{g_bits},
                                                                                          _b_bits{b_bits},
                                                                                          _a_bits{a_bits} {
            _pixels_count = _w * _h;
            _pixel_size_bits = r_bits + g_bits + b_bits + a_bits;
            _pixel_storage_size_bits = infer_power_of_2_bits_needed_from_bits(_pixel_size_bits);
            _mask = (uint64_t (1) << _pixel_storage_size_bits) - 1;
            uint size = (_pixel_storage_size_bits * _pixels_count) / buffer_element_size_bits;
            _data.resize(size);
        }

        void write(unsigned index, const color_t &color) override {
            const number packed = pack_bits_in_number(color);
            write(index, packed);
        }

        void write(unsigned index, uint64_t value) {
            // mask the value to the lower pixel storage bits window
            number value_masked = number(value) & _mask;
            // index of containing element inside the elements array
            unsigned int idx2 = (index * _pixel_storage_size_bits) / buffer_element_size_bits;
            // distance in bits from beginning of storage window to beginning of block
            unsigned int D = (index * _pixel_storage_size_bits) - idx2 * buffer_element_size_bits;
            // get the element to update
            number element_to_change = _data[idx2];
            // create a mask to extract zero the place where the pixel should lay
            // by moving the mask to the correct place
            const number clear_mask = (_mask) << (D);
            // clear the bits in the designated pixel position
            element_to_change &= (~clear_mask);
            // move the new value int place
            number new_element = (value_masked) << (D);
            // blend the element. the value was masked in the beginning with zeroes, so all good
            new_element = element_to_change | new_element;
            // write the modified element
            _data[idx2] = new_element;
        }

        color_t operator[](int index) const override {
            // todo: implement
            throw std::runtime_error{"implement me !!!"};
            return color_t{};
        }

        number pack_bits_in_number(const color_t &color) {
            ubyte rgba_bits = color.bits_rgba();
            ubyte number_bits = sizeof(number) << 3;
            if (rgba_bits > number_bits)
                throw std::runtime_error{"bad conversion"};
            number storage = (color.r << color.bits_gba()) +
                             (color.g << color.bits_ba()) +
                             (color.b << color.bits_a()) +
                             (color.a << 0);
            return storage;
        }

        str toString(const str &name) const override {
            const str type_name = infer_type_needed_for_bits(buffer_element_size_bits);
            const str space = " ";
            str var = type_name + space + name + "[" + std::to_string(_w) + "*" + std::to_string(_h) + "]= {";
            const auto buffer_size = _data.size();
            for (unsigned long ix = 0; ix < buffer_size; ++ix) {
                const auto hex_str = intToHexString<number>(_data[ix]);
                var += hex_str;
                if (ix < buffer_size - 1) { // not last one
                    var += ", ";
                }
            }
            var += "}";
            return var;
        }
    };

    class factory_PackedArray {
    public:
        factory_PackedArray()= delete;

        static
        addressable_ram * getArray(uint w, uint h,
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

}