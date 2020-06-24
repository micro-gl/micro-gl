#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma once

#include <microgl/base_bitmap.h>

template <typename P, typename CODER>
class Bitmap : public base_bitmap<Bitmap<P, CODER>, P, CODER> {
    using base=base_bitmap<Bitmap<P, CODER>, P, CODER>;
public:
    using base::pixelAt;
    using base::writeAt;

    template<typename P2, typename CODER2>
    Bitmap<P2, CODER2> * convertToBitmap() {
        auto * bmp_2 = new Bitmap<P2, CODER2>(this->_width, this->_height);
        copyToBitmap(*bmp_2);
        return bmp_2;
    }

    template<typename P2, typename CODER2>
    void copyToBitmap(Bitmap<P2, CODER2> & bmp) {
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

    P pixelAt(int index) const {
        return this->_buffer._data[index];
    }

    void writeAt(int index, const P &value) {
        this->_buffer.writeAt(value, index);
    }

    void fill(const P &value) {
        this->_buffer.fill(value);
    }

protected:
};

#pragma clang diagnostic pop