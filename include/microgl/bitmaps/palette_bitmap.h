/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "base_bitmap.h"
#include "../stdint.h"

/**
 * a bitmap that uses a palette of pixels of size 2, 4, 16, 256. This way,
 * the pixels array serves as indices, that can be packed as 1,2,4,8 bits
 * respectively. This is memory eficient.
 *
 * @tparam PALETTE_SIZE 2, 4, 16, 256 size
 * @tparam CODER the pixel coder for the palette
 * @tparam reverse_elements_pos_in_byte can help with endian-ness issues
 */
template <unsigned PALETTE_SIZE, typename CODER, bool reverse_elements_pos_in_byte=false, class allocator_type=microgl::traits::std_rebind_allocator<>>
class palette_bitmap : public base_bitmap<palette_bitmap<PALETTE_SIZE, CODER, reverse_elements_pos_in_byte, allocator_type>, allocator_type, CODER, microgl::ints::uint8_t> {
    using base=base_bitmap<palette_bitmap<PALETTE_SIZE, CODER, reverse_elements_pos_in_byte, allocator_type>, allocator_type, CODER, microgl::ints::uint8_t>;
    using byte=unsigned char;
    static constexpr byte BPI = PALETTE_SIZE==2 ? 1 : (PALETTE_SIZE==4 ? 2 : (PALETTE_SIZE==16 ? 4 : (PALETTE_SIZE==256 ? 8 : 0)));
    static constexpr bool is_1_2_4_8_bits = BPI!=0;
    typename microgl::traits::enable_if<is_1_2_4_8_bits, bool>::type fails_if_not_2_4_16_256_colors;
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

    void move_from(palette_bitmap & bmp) {
        owns_palette = bmp.owns_palette;
        bmp.owns_palette=false;
        palette = bmp.palette;
        bmp.palette= nullptr;
    }

    void copy_from(const palette_bitmap & bmp) {
        if(owns_palette) delete [] palette;
        owns_palette = true;
        palette = new pixel[PALETTE_SIZE];
        for (int ix = 0; ix < PALETTE_SIZE; ++ix)
            palette[ix] = bmp.palette[ix];
    }

    static int round(int val) {
        return (val + ((1<<T)-1))>>T;
    }

public:
    bool owns_palette = false;

    /**
     * construct a bitmap with a given indices array and pixel palette.
     * @param $indices the indices array
     * @param palette the palette
     * @param w the bitmap width
     * @param h the bitmap height
     */
    palette_bitmap(void* $indices, void * $palette, int w, int h,
                   const allocator_type & allocator=allocator_type()) :
                base{$indices, round(w*h), w, h, allocator},
                palette{reinterpret_cast<pixel *>($palette)} {
    };
    /**
     * construct a bitmap, allocate indices array.
     * @param w the bitmap width
     * @param h the bitmap height
     */
    palette_bitmap(int w, int h, const allocator_type & allocator=allocator_type()) :
                base{w, h, allocator} {};

    palette_bitmap(const palette_bitmap & bmp) : base{bmp} {
        copy_from(bmp);
    }
    palette_bitmap(palette_bitmap && bmp)  noexcept : base(microgl::traits::move(bmp)) {
        move_from(bmp);
    }
    palette_bitmap & operator=(const palette_bitmap & bmp) {
        if(this==&bmp) return *this;
        base::operator=(bmp);
        copy_from(bmp);
        return *this;
    }
    palette_bitmap & operator=(palette_bitmap && bmp) noexcept {
        base::operator=(microgl::traits::move(bmp));
        move_from(bmp);
        return *this;
    }
    ~palette_bitmap() {
        if(owns_palette) delete [] palette;
    }

    /**
     * get the palette size
     */
    constexpr unsigned paletteSize() const { return PALETTE_SIZE; }

    /**
     * update the current palette with another
     * @param $palette pixel palette
     */
    void updatePalette(pixel * $palette, bool owner=false) {
        owns_palette=owner;
        palette = $palette;
    }

    byte extract_pixel_index(unsigned int index1) const {
        byte mm=M, kk=K, tt=T, mask=MASK; // debug
        unsigned int idx2=(index1)>>T; // index inside the elements array
        byte element= this->_buffer[idx2]; // inside this element we need to extract the pixel
        unsigned int R=(index1<<K)-(idx2<<M); // compute distance to the beginning of the 8bit aligned block
        element= reverse_elements_pos_in_byte ? (element) >> (BPE - BPI - R) : (element) >> (R); // move the element to the lower part
        byte masked=element&(MASK); // mask out the upper bits
        return masked;
    }

    pixel pixelAt(int index) const { return palette[extract_pixel_index(index)]; }

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
        byte byte_to_change=this->_buffer[idx2];
        clear_mask= reverse_elements_pos_in_byte ? // move the mask to the correct position
                (clear_mask) << (BPE - BPI - R) : (clear_mask) << (R);
        byte_to_change &= (~clear_mask); // clear the bits in the designated pixel position
        byte element= reverse_elements_pos_in_byte ? // move the value to the correct position
                (masked_value) << (BPE - BPI - R) : (masked_value) << (R);
        element = byte_to_change | element; // not merge/blend the bits
        this->_buffer[idx2] = element; // record
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
