#pragma once

#include <microgl/base_bitmap.h>

/**
 * a bitmap that uses a palette of pixels of size 2, 4, 16, 256. This way,
 * the pixels array serves as indices, that can be packed as 1,2,4,8 bits
 * respectively. This is memory eficient.
 *
 * @tparam PALETTE_SIZE 2, 4, 16, 256 size
 * @tparam CODER the pixel coder for the palette
 * @tparam reverse_elements_pos_in_byte can help with endian-ness issues
 */
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
    using pixel=typename base::pixel;

private:
    pixel * palette = nullptr;
public:

    static
    int pad_to(int val, int bits, int align_bits=8) {
        int I= (val*bits) % align_bits;
        int R=align_bits-I;
        int extra=R/bits;
        return val+extra;
    }

    /**
     * construct a bitmap with a given indices array and pixel palette.
     * @param $indices the indices array
     * @param palette the palette
     * @param w the bitmap width
     * @param h the bitmap height
     */
    PaletteBitmap(void* $indices, void *palette, int w, int h) :
                base{$indices, (w*h + ((1<<T)-1))>>T, w, h},
                palette{reinterpret_cast<pixel *>(palette)} {
    };
    /**
     * construct a bitmap, allocate indices array.
     * @param w the bitmap width
     * @param h the bitmap height
     */
    PaletteBitmap(int w, int h) : PaletteBitmap{new uint8_t[(w*h + ((1<<T)-1))>>T],
                                                new pixel[PALETTE_SIZE], w, h} {};
    ~PaletteBitmap() = default;

    /**
     * get the palette size
     */
    unsigned paletteSize() { return PALETTE_SIZE; }

    /**
     * update the current palette with another
     * @param $palette pixel palette
     */
    void updatePalette(pixel * $palette) {
        palette = $palette;
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

    pixel pixelAt(int index) const {
        return palette[extract_pixel_index(index)];
    }

    int locate_index_color_of_pixel_in_palette(const pixel & pixel) {
        for (int ix = 0; ix < int(PALETTE_SIZE); ++ix) {
            if (palette[ix] == pixel)
                return ix;
        }
        return -1;
    }

    void writeAt(int index1, const pixel &value) {
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

    void fill(const pixel &value) {
        // fast fill
        byte masked=locate_index_color_of_pixel_in_palette(value);
        byte byte_rendered=0;
        auto pixels_per_byte=(1u<<T);
        for (unsigned pos = 0; pos < pixels_per_byte; ++pos)
            byte_rendered |= (masked<<(BPI * pos));
        this->_buffer.fill(byte_rendered);
    }

};
