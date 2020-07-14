#pragma once

#include <addressable_ram.h>
#include <bits.h>
#include <utils.h>
#include <cmath>
#include <exception>

namespace imagium {
    
    template<typename number>
    class typed_packed_array : public addressable_ram {
        std::vector<number> _data;
        const uint _pixels_count;
        const uint _pixel_storage_size_bits = 0; // power of 2
        const unsigned buffer_element_size_bits = sizeof(number) * 8;
        number _mask;
    public:
        explicit typed_packed_array(uint pixels, uint pixel_storage_size_bits) : _pixels_count{pixels},
                                                                                 _pixel_storage_size_bits{pixel_storage_size_bits}
                                                                                           {
            _mask = (uint64_t (1) << _pixel_storage_size_bits) - 1;
            uint size = std::ceil(float(_pixel_storage_size_bits * _pixels_count) / buffer_element_size_bits);
            _data.resize(size);
        }

        void write(unsigned index, uint64_t value) override {
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

        uint64_t operator[](int index) const override {
            // todo: implement
            throw std::runtime_error{"implement me !!!"};
        }


        str toString(const str &name) const override {
            const str type_name = infer_type_needed_for_bits(buffer_element_size_bits);
            const str space = " ";
            str var = type_name + space + name + "[" + std::to_string(_data.size()) + "]= {";
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
        addressable_ram * getArray(uint size, uint pixel_storage_size_bits) {
            const unsigned bytes_per_element=infer_power_of_2_bytes_needed_from_bits(pixel_storage_size_bits);
            switch (bytes_per_element) {
                case 1: return new typed_packed_array<uint8_t>{size, pixel_storage_size_bits};
                case 2: return new typed_packed_array<uint16_t>{size, pixel_storage_size_bits};
                case 4: return new typed_packed_array<uint32_t>{size, pixel_storage_size_bits};
                case 8: return new typed_packed_array<uint64_t>{size, pixel_storage_size_bits};
                default: return new typed_packed_array<uint64_t>{size, pixel_storage_size_bits};
            }
        }

    };

}