#pragma once

#include <microgl/base_bitmap.h>

template <typename pixel_coder_>
class bitmap : public base_bitmap<bitmap<pixel_coder_>, pixel_coder_> {
    using base=base_bitmap<bitmap<pixel_coder_>, pixel_coder_>;
public:
    using base::pixelAt;
    using base::writeAt;
    using pixel=typename base::pixel;

    template<typename CODER2>
    bitmap<CODER2> * convertToBitmap() {
        auto * bmp_2 = new bitmap<CODER2>(this->_width, this->_height);
        copyToBitmap(*bmp_2);
        return bmp_2;
    }

    template<typename CODER2>
    void copyToBitmap(bitmap<CODER2> & bmp) {
        if(bmp.size()!=this->size()) return;
        const int size = this->size();
        microgl::color::color_t color_bmp_1, color_bmp_2;
        for (int index = 0; index < size; ++index) {
            this->decode(index, color_bmp_1);
//            this->coder().template convert<CODER2>(color_bmp_1, color_bmp_2);
            microgl::color::convert_color<typename base::rgba, typename CODER2::rgba>(color_bmp_1, color_bmp_2);

            bmp.writeColor(index, color_bmp_2);
        }
    }

    bitmap(int w, int h) : base{w,h} {};
    bitmap(uint8_t* $pixels, int w, int h) : base {$pixels, w, h} {};
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
