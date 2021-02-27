#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma once

#include <microgl/base_bitmap.h>

template <unsigned PALETTE_SIZE, typename CODER, bool reverse_elements_pos_in_byte=false>
class PaletteBitmap : public base_bitmap<PaletteBitmap<PALETTE_SIZE, CODER, reverse_elements_pos_in_byte>, CODER, uint8_t> {
    using base=base_bitmap<PaletteBitmap<PALETTE_SIZE, CODER, reverse_elements_pos_in_byte>, CODER, uint8_t>;
    using byte=unsigned char;
    static constexpr byte BPI = PALETTE_SIZE==2 ? 1 : (PALETTE_SIZE==4 ? 2 : (PALETTE_SIZE==16 ? 4 : (PALETTE_SIZE==256 ? 8 : 0)));
    static constexpr bool is_1_2_4_8_bits = BPI!=0;
    typename std::enable_if<is_1_2_4_8_bits, bool>::type fails_if_not_2_4_16_256_colors;
    static constexpr byte M = 3; // always <= 3
    static constexpr byte BPE = byte(1)<<M; // always 8 bits, 1 byte
    static constexpr byte K=(BPI == 1 ? 0 : (BPI == 2 ? 1 : (BPI == 4 ? 2 : (BPI == 8 ? 3 : 4))));
    static constexpr byte T=M-K;
    static constexpr byte MASK=(BPI == 1 ? 0b00000001 : (BPI == 2 ? 0b00000011 :
                            (BPI == 4 ? 0b00001111 : (BPI == 8 ? 0b11111111 : 0b11111111))));
public:
    using base::pixelAt;
    using base::writeAt;
    using Pixel=typename base::Pixel;

    Pixel palette[PALETTE_SIZE];

    static
    int pad_to(int val, int bits, int align_bits=8) {
        int I= (val*bits) % align_bits;
        int R=align_bits-I;
        int extra=R/bits;
        return val+extra;
    }

    PaletteBitmap(int w, int h) : PaletteBitmap{new uint8_t[(w*h)>>T], nullptr, w, h} {};
    PaletteBitmap(uint8_t* $indices, const Pixel *palette, int w, int h) : base {$indices, (w*h)>>T, w, h} {
        updatePalette(palette);
    };
    PaletteBitmap(uint8_t* $indices, const void *palette, int w, int h) :
                            PaletteBitmap{$indices, reinterpret_cast<const Pixel *>(palette), w, h} {};
    ~PaletteBitmap() = default;

    void updatePaletteValue(const byte& index, const Pixel & value) {
        palette[index]=value;
    }

    unsigned paletteSize() { return PALETTE_SIZE; }

    void updatePalette(const Pixel *palette) {
        for (unsigned ix = 0; ix < PALETTE_SIZE; ++ix) updatePaletteValue(ix, palette[ix]);
    }

    byte extract_pixel_index(unsigned int index1) const {
        byte mm=M, kk=K, tt=T, mask=MASK; // debug
        unsigned int idx2=(index1)>>T; // index inside the elements array
        byte element= this->_buffer._data[idx2]; // inside this element we need to extract the pixel
        unsigned int R=(index1<<K)-(idx2<<M); // compute distance to the beginning of the 8bit aligned block
        element= reverse_elements_pos_in_byte ? (element) >> (BPE - BPI - R) : (element) >> (R); // move the element to the lower part
        byte masked=element&(MASK); // mask out the upper bits
        return masked;
    }

    Pixel pixelAt(int index) const {
        return palette[extract_pixel_index(index)];
    }

    int locate_index_color_of_pixel_in_palette(const Pixel & pixel) {
        for (int ix = 0; ix < int(PALETTE_SIZE); ++ix) {
            if (palette[ix] == pixel)
                return ix;
        }
        return -1;
    }

    void writeAt(int index1, const Pixel &value) {
        // warning:: very slow method
        byte color = locate_index_color_of_pixel_in_palette(value);
        byte mm=M, kk=K, tt=T, mask=MASK; // debug
        byte masked_value=color&MASK; // mask the value, this is redundant
        byte clear_mask=MASK; // to clear
        unsigned int idx2=(index1)>>T; // index inside the elements array
        unsigned int R=(index1<<K)-(idx2<<M); // compute distance to the beginning of the block
        byte byte_to_change=this->_buffer._data[idx2];
        clear_mask= reverse_elements_pos_in_byte ? // move the mask to the correct position
                (clear_mask) << (BPE - BPI - R) : (clear_mask) << (R);
        byte_to_change &= (~clear_mask); // clear the bits in the designated pixel position
        byte element= reverse_elements_pos_in_byte ? // move the value to the correct position
                (masked_value) << (BPE - BPI - R) : (masked_value) << (R);
        element = byte_to_change | element; // not merge/blend the bits
        this->_buffer._data[idx2] = element; // record
    }

    void fill(const Pixel &value) {
        // fast fill
        byte masked=locate_index_color_of_pixel_in_palette(value);
        byte byte_rendered=0;
        auto pixels_per_byte=(1u<<T);
        for (unsigned pos = 0; pos < pixels_per_byte; ++pos)
            byte_rendered |= (masked<<(BPI * pos));
        this->_buffer.fill(byte_rendered);
    }

};

#pragma clang diagnostic pop