#pragma once

#include <microgl/base_bitmap.h>

template <typename CODER>
class Bitmap : public base_bitmap<Bitmap<CODER>, CODER> {
    using base=base_bitmap<Bitmap<CODER>, CODER>;
public:
    using base::pixelAt;
    using base::writeAt;
    using Pixel=typename base::Pixel;

    template<typename CODER2>
    Bitmap<CODER2> * convertToBitmap() {
        auto * bmp_2 = new Bitmap<CODER2>(this->_width, this->_height);
        copyToBitmap(*bmp_2);
        return bmp_2;
    }

    template<typename CODER2>
    void copyToBitmap(Bitmap<CODER2> & bmp) {
        if(bmp.size()!=this->size()) return;
        const int size = this->size();
        microgl::color::color_t color_bmp_1, color_bmp_2;
        for (int index = 0; index < size; ++index) {
            this->decode(index, color_bmp_1);
            this->coder().convert(color_bmp_1, color_bmp_2, bmp.coder());
            bmp.writeColor(index, color_bmp_2);
        }
    }

    Bitmap(int w, int h) : base{w,h} {};
    Bitmap(uint8_t* $pixels, int w, int h) : base {$pixels, w, h} {};
    ~Bitmap() = default;

    Pixel pixelAt(int index) const {
        return this->_buffer._data[index];
    }

    void writeAt(int index, const Pixel &value) {
        this->_buffer.writeAt(value, index);
    }

    void fill(const Pixel &value) {
        this->_buffer.fill(value);
    }

protected:
};
