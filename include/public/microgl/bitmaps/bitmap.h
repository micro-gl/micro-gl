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
 * regular bitmap
 *
 * @tparam pixel_coder_ the pixel coder
 */
template <typename pixel_coder_, class allocator_type=microgl::traits::std_rebind_allocator<>>
class bitmap : public base_bitmap<bitmap<pixel_coder_, allocator_type>, allocator_type, pixel_coder_> {
    using base=base_bitmap<bitmap<pixel_coder_, allocator_type>, allocator_type, pixel_coder_>;
public:
    using base::pixelAt;
    using base::writeAt;
    using pixel=typename base::pixel;

    /**
     * convert this bitmap into a new bitmap
     *
     * @tparam CODER2 the coder of the new bitmap
     *
     * @return the new bitmap
     */
    template<typename CODER2>
    bitmap<CODER2, allocator_type> * convertToBitmap() {
        auto * bmp_2 = new bitmap<CODER2, allocator_type>(this->_width, this->_height);
        copyToBitmap(*bmp_2);
        return bmp_2;
    }

    /**
     * copy this bitmap to another bitmap
     *
     * @tparam CODER2 the coder of the new bitmap
     *
     * @param bmp the other bitmap reference
     */
    template<typename CODER2>
    void copyToBitmap(bitmap<CODER2, allocator_type> & bmp) {
        if(bmp.size()!=this->size()) return;
        const int size = this->size();
        microgl::color_t color_bmp_1, color_bmp_2;
        for (int index = 0; index < size; ++index) {
            this->decode(index, color_bmp_1);
            microgl::convert_color<typename base::rgba, typename CODER2::rgba>(
                    color_bmp_1, color_bmp_2);
            bmp.writeColor(index, color_bmp_2);
        }
    }

    /**
     * create a new bitmap and allocate pixel array
     * @param w width of bitmap
     * @param h height of bitmap
     */
    bitmap(int w, int h, const allocator_type & allocator=allocator_type()) : base{w, h, allocator} {};
    /**
     * create a new bitmap with a given pixel array
     * @param $pixels
     * @param w width of bitmap
     * @param h height of bitmap
     */
    bitmap(void *$pixels, int w, int h, const allocator_type & allocator=allocator_type()) : base{$pixels, w, h, allocator} {}
    bitmap(const bitmap & bmp) : base{bmp} {}
    bitmap(bitmap && bmp)  noexcept : base(microgl::traits::move(bmp)) {}
    bitmap & operator=(const bitmap & bmp) {
        base::operator=(bmp);
        return *this;
    }
    bitmap & operator=(bitmap && bmp) noexcept {
        base::operator=(microgl::traits::move(bmp));
        return *this;
    }
    ~bitmap() = default;

    pixel pixelAt(int index) const { return this->_buffer[index]; }
    void writeAt(int index, const pixel &value) { this->_buffer.writeAt(value, index); }
    void fill(const pixel &value) { this->_buffer.fill(value); }
};
