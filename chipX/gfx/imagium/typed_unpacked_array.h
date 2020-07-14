#pragma once

#include <addressable_ram.h>
#include <bits.h>
#include <utils.h>
#include <exception>

namespace imagium {
    
    template<typename number>
    class typed_unpacked_array : public addressable_ram {
        std::vector<number> _data;
        const uint _pixels_count;
        const bits _r_bits, _g_bits, _b_bits, _a_bits;
        uint _channels_count = 0;
        const unsigned channel_size_bits = sizeof(number) * 8;
        const unsigned buffer_element_size_bits = sizeof(number) * 8;
        number _mask;
    public:
        bool hasRedChannel() const { return _r_bits!=0; }
        bool hasGreenChannel() const { return _g_bits!=0; }
        bool hasBlueChannel() const { return _b_bits!=0; }
        bool hasAlphaChannel() const { return _a_bits!=0; }
        uint channelsCount() const {
            uint result=0;
            if(hasRedChannel()) ++result;
            if(hasGreenChannel()) ++result;
            if(hasBlueChannel()) ++result;
            if(hasAlphaChannel()) ++result;
            return result;
        }
        explicit typed_unpacked_array(uint pixels,
                                    bits r_bits, bits g_bits, bits b_bits, bits a_bits) : _pixels_count{pixels},
                                                                                          _r_bits{r_bits},
                                                                                          _g_bits{g_bits},
                                                                                          _b_bits{b_bits},
                                                                                          _a_bits{a_bits} {
            _channels_count= channelsCount();
            _data.resize(_pixels_count*_channels_count);
        }

        void write(unsigned index, const color_t &color) override {
            unsigned idx2=index*_channels_count;
            unsigned acc=0;
            if(hasRedChannel())     _data[idx2+acc++]=color.r;
            if(hasGreenChannel())   _data[idx2+acc++]=color.g;
            if(hasBlueChannel())    _data[idx2+acc++]=color.b;
            if(hasAlphaChannel())   _data[idx2+acc++]=color.a;
        }

        void write(unsigned index, uint64_t value) override {

        }

        color_t operator[](int index) const override {
            color_t color;
            unsigned idx2=index*_channels_count, acc=0;
            if(hasRedChannel())     color.r=_data[idx2+acc++];
            if(hasGreenChannel())   color.g=_data[idx2+acc++];
            if(hasBlueChannel())    color.b=_data[idx2+acc++];
            if(hasAlphaChannel())   color.a=_data[idx2+acc++];
            return color;
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

    class factory_UnpackedArray {
    public:
        factory_UnpackedArray()= delete;

        static
        addressable_ram * getArray(uint size, bits r_bits, bits g_bits, bits b_bits, bits a_bits) {
            bits max_bits=std::max<bits>({r_bits, g_bits, b_bits, a_bits});
            ubyte needed_power_of_2_pixel_bits=infer_power_of_2_bits_needed_from_bits(max_bits);
            const unsigned bytes_per_element=infer_power_of_2_bytes_needed_from_bits(needed_power_of_2_pixel_bits);
            switch (bytes_per_element) {
                case 1: return new typed_unpacked_array<uint8_t>{size, r_bits, g_bits, b_bits, a_bits};
                case 2: return new typed_unpacked_array<uint16_t>{size, r_bits, g_bits, b_bits, a_bits};
                case 4: return new typed_unpacked_array<uint32_t>{size, r_bits, g_bits, b_bits, a_bits};
                case 8: return new typed_unpacked_array<uint64_t>{size, r_bits, g_bits, b_bits, a_bits};
                default: return new typed_unpacked_array<uint64_t>{size, r_bits, g_bits, b_bits, a_bits};
            }
        }

    };

}