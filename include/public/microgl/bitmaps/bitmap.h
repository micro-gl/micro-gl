#pragma once

#include <microgl/base_bitmap.h>

/**
 * regular bitmap
 *
 * @tparam pixel_coder_ the pixel coder
 */
template <typename pixel_coder_>
class bitmap : public base_bitmap<bitmap<pixel_coder_>, pixel_coder_> {
    using base=base_bitmap<bitmap<pixel_coder_>, pixel_coder_>;
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
    bitmap<CODER2> * convertToBitmap() {
        auto * bmp_2 = new bitmap<CODER2>(this->_width, this->_height);
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
    void copyToBitmap(bitmap<CODER2> & bmp) {
        if(bmp.size()!=this->size()) return;
        const int size = this->size();
        microgl::color::color_t color_bmp_1, color_bmp_2;
        for (int index = 0; index < size; ++index) {
            this->decode(index, color_bmp_1);
//            this->coder().template convert<CODER2>(color_bmp_1, color_bmp_2);
            microgl::color::convert_color<typename base::rgba, typename CODER2::rgba>(
                    color_bmp_1, color_bmp_2);
            bmp.writeColor(index, color_bmp_2);
        }
    }

    /**
     * create a new bitmap and allocate pixel array
     * @param w width of bitmap
     * @param h height of bitmap
     */
    bitmap(int w, int h) : base{w,h} {};
    /**
     * create a new bitmap with a given pixel array
     * @param $pixels
     * @param w width of bitmap
     * @param h height of bitmap
     */
    bitmap(void *$pixels, int w, int h) : base{$pixels, w, h} {}
    ~bitmap() = default;

    pixel pixelAt(int index) const {
        return this->_buffer._data[index];
    }

    void writeAt(int index, const pixel &value) {
        this->_buffer.writeAt(value, index);
    }

    void fill(const pixel &value) {
        this->_buffer.fill(value);
    }

protected:
};
