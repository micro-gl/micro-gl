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

/**
 * packed bitmap is a memory efficient bitmap, that encodes pixels as group of bits
 * of size 1, 2, 4, 8 bits. This is good for single channel storage like text.
 *
 * @tparam BPP bits per pixel (1|2|4|8)
 * @tparam CODER the pixel coder to decode the pixels
 * @tparam reverse_elements_pos_in_byte this can help with endian-ness issues
 */
template <unsigned BPP, typename CODER, bool reverse_elements_pos_in_byte=false, class allocator_type=microgl::traits::std_rebind_allocator<>>
class packed_bitmap : public base_bitmap<packed_bitmap<BPP, CODER, reverse_elements_pos_in_byte, allocator_type>, allocator_type, CODER, microgl::ints::uint8_t> {
    using base=base_bitmap<packed_bitmap<BPP, CODER, reverse_elements_pos_in_byte>, allocator_type, CODER, microgl::ints::uint8_t>;
    using byte=unsigned char;
    static constexpr bool is_1_2_4_8 = BPP==1||BPP==2||BPP==4||BPP==8;
    typename microgl::traits::enable_if<is_1_2_4_8, bool>::type fails_if_else;
    typename microgl::traits::enable_if<microgl::traits::is_same<typename CODER::pixel, byte>::value, bool>::type fails_if_not_pixel_8_bit;
    static constexpr byte M = 3;
    static constexpr byte BPE = byte(1)<<M; // always 8 bits, 1 byte
    static constexpr byte K=(BPP==1 ? 0 : (BPP==2 ? 1 : (BPP==4 ? 2 : (BPP==8 ? 3 : 4))));
    static constexpr byte T=M-K;
    static constexpr byte MASK=(BPP==1 ? 0b00000001 : (BPP==2 ? 0b00000011 : (BPP==4 ? 0b00001111 : 0b11111111)));

public:
    using pixel = typename base::pixel;
    using base::pixelAt;
    using base::writeAt;

    static int round(int val) {
        return (val + ((1<<T)-1))>>T;
    }

    /**
     * construct a bitmap with a given pixel array
     *
     * @param $pixels the pixels array
     * @param w the bitmap width
     * @param h the bitmap height
     */
    packed_bitmap(void* $pixels, int w, int h,
                  const allocator_type & allocator=allocator_type()) :
                 base {$pixels, round(w*h), w, h, allocator} {};
    /**
     * construct a bitmap and allocate a pixel array
     *
     * @param w the bitmap width
     * @param h the bitmap height
     */
    packed_bitmap(int w, int h, const allocator_type & allocator=allocator_type()) :
                    base{w, h, allocator} {};
    packed_bitmap(const packed_bitmap & bmp) : base{bmp} {}
    packed_bitmap(packed_bitmap && bmp)  noexcept : base(microgl::traits::move(bmp)) {}
    packed_bitmap & operator=(const packed_bitmap & bmp) {
        base::operator=(bmp);
        return *this;
    }
    packed_bitmap & operator=(packed_bitmap && bmp) noexcept {
        base::operator=(microgl::traits::move(bmp));
        return *this;
    }
    ~packed_bitmap() = default;

    microgl::ints::uint8_t extract_pixel(unsigned int index1) const {
        byte mm=M, kk=K, tt=T, mask=MASK; // debug
        unsigned int idx2=(index1)>>T; // index inside the elements array
        byte element= this->_buffer[idx2]; // inside this element we need to extract the pixel
        unsigned int R=(index1<<K)-(idx2<<M); // compute distance to the beginning of the 8bit aligned block
        element= reverse_elements_pos_in_byte ? (element) >> (BPE - BPP - R) : (element) >> (R); // move the element to the lower part
        byte masked=element&(MASK); // mask out the upper bits
        return masked;
//        return masked<<4;
//        return masked<<6;
//        return masked<<7;
    }

    microgl::ints::uint8_t pixelAt(int index) const { return extract_pixel(index); }
    void writeAt(int index1, const microgl::ints::uint8_t &value) {
        // todo:: measure performance on this method
        byte mm=M, kk=K, tt=T, mask=MASK; // debug
        byte masked_value=value&MASK; // mask the value
        byte clear_mask=MASK; // to clear
        unsigned int idx2=(index1)>>T; // index inside the elements array
        unsigned int R=(index1<<K)-(idx2<<M); // compute distance to the beginning of the block
        byte byte_to_change=this->_buffer[idx2];
        clear_mask= reverse_elements_pos_in_byte ? // move the mask to the correct position
                (clear_mask) << (BPE - BPP - R) : (clear_mask) << (R);
        byte_to_change &= (~clear_mask); // clear the bits in the designated pixel position
        byte element= reverse_elements_pos_in_byte ? // move the value to the correct position
                (masked_value) << (BPE - BPP - R) : (masked_value) << (R);
        element = byte_to_change | element; // not merge/blend the bits
        this->_buffer[idx2] = element; // record
    }

    void fill(const microgl::ints::uint8_t &value) {
        // fast fill
        byte masked=value&MASK;
        byte byte_rendered=0;
        auto pixels_per_byte=(1u<<T);
        for (unsigned pos = 0; pos < pixels_per_byte; ++pos)
            byte_rendered |= (masked<<(BPP*pos));
        this->_buffer.fill(byte_rendered);
    }
};
