#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma once

#include <microgl/base_bitmap.h>
#include <microgl/micro_gl_traits.h>

template <unsigned BPP, typename CODER, bool reverse_elements_pos_in_byte=false>
class PackedBitmap : public base_bitmap<PackedBitmap<BPP, CODER, reverse_elements_pos_in_byte>, CODER, uint8_t> {
    using base=base_bitmap<PackedBitmap<BPP, CODER, reverse_elements_pos_in_byte>, CODER, uint8_t>;
    using byte=unsigned char;
    static constexpr bool is_1_2_4_8 = BPP==1||BPP==2||BPP==4||BPP==8;
    typename std::enable_if<is_1_2_4_8, bool>::type fails_if_else;
    typename std::enable_if<microgl::traits::is_same<typename CODER::Pixel, byte>::value, bool>::type fails_if_not_pixel_8_bit;
    static constexpr byte M = 3;
    static constexpr byte BPE = byte(1)<<M; // always 8 bits, 1 byte
    static constexpr byte K=(BPP==1 ? 0 : (BPP==2 ? 1 : (BPP==4 ? 2 : (BPP==8 ? 3 : 4))));
    static constexpr byte T=M-K;
    static constexpr byte MASK=(BPP==1 ? 0b00000001 : (BPP==2 ? 0b00000011 : (BPP==4 ? 0b00001111 : 0b11111111)));

public:
    using base::pixelAt;
    using base::writeAt;

    static
    int pad_to(int val, int bits, int align) {
        int I=(val*bits)%align;
        int R=8-I;
        int extra=R/bits;
        return val+extra;
    }

    PackedBitmap(int w, int h) : PackedBitmap{new uint8_t[(w*h)>>T], w, h} {};
    PackedBitmap(uint8_t* $pixels, int w, int h) : base {$pixels, (w*h)>>T, w, h} {};
    ~PackedBitmap() = default;

    uint8_t extract_pixel(unsigned int index1) const {
        byte mm=M, kk=K, tt=T, mask=MASK; // debug
        unsigned int idx2=(index1)>>T; // index inside the elements array
        byte element= this->_buffer._data[idx2]; // inside this element we need to extract the pixel
        unsigned int R=(index1<<K)-(idx2<<M); // compute distance to the beginning of the 8bit aligned block
        element= reverse_elements_pos_in_byte ? (element) >> (BPE - BPP - R) : (element) >> (R); // move the element to the lower part
        byte masked=element&(MASK); // mask out the upper bits
//        return masked;
        return masked<<4;
//        return masked<<6;
//        return masked<<7;
    }

    uint8_t pixelAt(int index) const {
        return extract_pixel(index);
    }

    void writeAt(int index1, const uint8_t &value) {
        // todo:: measure performance on this method
        byte mm=M, kk=K, tt=T, mask=MASK; // debug
        byte masked_value=value&MASK; // mask the value
        byte clear_mask=MASK; // to clear
        unsigned int idx2=(index1)>>T; // index inside the elements array
        unsigned int R=(index1<<K)-(idx2<<M); // compute distance to the beginning of the block
        byte byte_to_change=this->_buffer._data[idx2];
        clear_mask= reverse_elements_pos_in_byte ? // move the mask to the correct position
                (clear_mask) << (BPE - BPP - R) : (clear_mask) << (R);
        byte_to_change &= (~clear_mask); // clear the bits in the designated pixel position
        byte element= reverse_elements_pos_in_byte ? // move the value to the correct position
                (masked_value) << (BPE - BPP - R) : (masked_value) << (R);
        element = byte_to_change | element; // not merge/blend the bits
        this->_buffer._data[idx2] = element; // record
    }

    void fill(const uint8_t &value) {
        // fast fill
        byte masked=value&MASK;
        byte byte_rendered=0;
        auto pixels_per_byte=(1u<<T);
        for (unsigned pos = 0; pos < pixels_per_byte; ++pos)
            byte_rendered |= (masked<<(BPP*pos));
        this->_buffer.fill(byte_rendered);
    }

};

#pragma clang diagnostic pop