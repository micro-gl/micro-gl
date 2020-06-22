#pragma once

#include <microgl/FrameBuffer.h>
#include <microgl/crpt.h>
#include <microgl/PixelCoder.h>

template <typename IMPL, typename P, typename CODER>
class base_bitmap : public crpt<IMPL> {
protected:
    int _width = 0, _height = 0;
    microgl::coder::PixelCoder<P, CODER> _coder;
    FrameBuffer<P> _buffer;

public:
//    template <typename P2, typename CODER2>
//    Bitmap<P2, CODER2> * convertToBitmap();
//
//    template <typename P2, typename CODER2>
//    void copyToBitmap(Bitmap<P2, CODER2> & bmp);

//    Bitmap(int w, int h);
//    Bitmap(P* $pixels, int w, int h);
//    Bitmap(uint8_t* $pixels, int w, int h);
//    ~Bitmap();

    static
    constexpr bool hasNativeAlphaChannel() {
        return CODER::alpha_bits()!=0;
    }

    static
    constexpr bool nativeAlphaChannelBits() {
        return hasNativeAlphaChannel() ? CODER::alpha_bits() : 8;
    }

    static
    constexpr int maxNativeAlphaChannelValue() {
        return (1<<nativeAlphaChannelBits())-1;
    }

//    template<typename P2, typename CODER2>
//    Bitmap<P2, CODER2> * convertToBitmap() {
//        auto * bmp_2 = new Bitmap<P2, CODER2>(_width, _height);
//        copyToBitmap(*bmp_2);
//        return bmp_2;
//    }
//
//    template<typename P2, typename CODER2>
//    void Bitmap<P, CODER>::copyToBitmap(Bitmap<P2, CODER2> & bmp) {
//        if(bmp.size()!=this->size()) return;
//        const int size = this->size();
//        microgl::color::color_t color_bmp_1, color_bmp_2;
//        for (int index = 0; index < size; ++index) {
//            this->decode(index, color_bmp_1);
//            coder().convert(color_bmp_1, color_bmp_2, bmp.coder());
//            bmp.writeColor(index, color_bmp_2);
//        }
//    }

//template<typename P, typename CODER>
//Bitmap<P, CODER>::Bitmap(P* $pixels, int w, int h) :
//        FrameBuffer<P>($pixels, w * h), _width{w}, _height{h} {
//}

//template<typename P, typename CODER>
//Bitmap<P, CODER>::Bitmap(int w, int h) :
//        Bitmap<P, CODER>::Bitmap(new P[w * h], w, h) {
//}

    base_bitmap(int w, int h) :
            base_bitmap(new uint8_t[sizeof(P) * w * h], w, h) {
    }

    base_bitmap(uint8_t *$pixels, int w, int h) :
            _buffer(reinterpret_cast<P *>($pixels), w * h), _width{w}, _height{h} {
    }

    ~base_bitmap() {
        _width = 0;
        _height = 0;
    }

    int width() const {
        return _width;
    }

    int height() const {
        return _height;
    }

    int size() {return width()*height();}

    P * data() {
        return _buffer._data;
    }

    P pixelAt(int x, int y) const {
        return this->derived().pixelAt(x, y);
    }
    P pixelAt(int index) const {
        return this->derived().pixelAt(index);
    }
    void writeAt(int x, int y, const P &value) {
        this->derived().writeAt(x, y, value);
    }
    void writeAt(int index, const P &value) {
        this->derived().writeAt(index, value);
    }
    void fill(const P &value) {
        this->derived().fill(value);
    }

    const microgl::coder::PixelCoder<P, CODER> &coder() const {
        return _coder;
    }

    void decode(int x, int y, microgl::color::color_t &output)  const{
        _coder.decode(pixelAt(x, y), output);
    }

    void decode(int index, microgl::color::color_t &output)  const{
        _coder.decode(pixelAt(index), output);
    }

    template <typename number>
    void decode(int x, int y, microgl::color::intensity<number> &output) const {
        _coder.decode(pixelAt(x, y), output);
    }

    template <typename number>
    void decode(int index, microgl::color::intensity<number> &output) const {
        _coder.decode(pixelAt(index), output);
    }

    void writeColor(int index, const microgl::color::color_t &color) {
        P output;
        _coder.encode(color, output);
        writeAt(index, output);
//        this->_buffer->_data[index] = output;
    }

    void writeColor(int x, int y, const microgl::color::color_t &color) {
        writeColor(y*_width + x, color);
    }

    template <typename number>
    void writeColor(int index, const microgl::color::intensity<number> &color) {
        P output;
        _coder.encode(color, output);
        writeAt(index, output);
//        this->_data[index] = output;
    }

    template <typename number>
    void writeColor(int x, int y, const microgl::color::intensity<number> &color) {
        writeColor(y*_width + x, color);
    }

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

};

//#include "../../src/Bitmap.tpp"