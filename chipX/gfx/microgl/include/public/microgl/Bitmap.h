#pragma once

#include <microgl/base_bitmap.h>

template <typename P, typename CODER>
class Bitmap : public base_bitmap<Bitmap<P, CODER>, P, CODER> {
    using base=base_bitmap<Bitmap<P, CODER>, P, CODER>;
public:
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
//    Bitmap(P* $pixels, int w, int h);
    Bitmap(uint8_t* $pixels, int w, int h) : base {$pixels, w, h} {

    };

    ~Bitmap() {

    }

    P pixelAt(int x, int y) const {
        return this->_buffer._data[y*this->_width + x];
    }

    P pixelAt(int index) const {
        return this->_buffer._data[index];
    }

    void writeAt(int x, int y, const P &value) {
        this->_buffer.writeAt(value, y*this->_width + x);
    }
    void writeAt(int index, const P &value) {
        this->_buffer.writeAt(value, index);
    }

    void fill(const P &value) {
        this->_buffer.fill(value);
    }

//
//    static
//    constexpr bool hasNativeAlphaChannel() {
//        return CODER::alpha_bits()!=0;
//    }
//
//    static
//    constexpr bool nativeAlphaChannelBits() {
//        return hasNativeAlphaChannel() ? CODER::alpha_bits() : 8;
//    }
//
//    static
//    constexpr int maxNativeAlphaChannelValue() {
//        return (1<<nativeAlphaChannelBits())-1;
//    }
//
//    P pixelAt(int x, int y) const;
//    P pixelAt(int index) const;
//    // decoders
//    void decode(int x, int y, microgl::color::color_t &output) const;
//    void decode(int index, microgl::color::color_t &output) const;
//    template <typename number>
//    void decode(int x, int y, microgl::color::intensity<number> &output) const;
//    template <typename number>
//    void decode(int index, microgl::color::intensity<number> &output) const;
//    // encoders
//    void writeColor(int index, const microgl::color::color_t & color);
//    void writeColor(int x, int y, const microgl::color::color_t & color);
//    template <typename number>
//    void writeColor(int index, const microgl::color::intensity<number> & color);
//    template <typename number>
//    void writeColor(int x, int y, const microgl::color::intensity<number> & color);
//    int width() const;
//    int height() const;
//    const microgl::coder::PixelCoder<P, CODER> & coder() const;

protected:
//    int _width = 0, _height = 0;
//    microgl::coder::PixelCoder<P, CODER> _coder;
};

#include "../../src/Bitmap.tpp"