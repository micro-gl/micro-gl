#pragma once

#include <microgl/buffer.h>
#include <microgl/crpt.h>
#include <microgl/PixelCoder.h>

template <typename IMPL, typename P, typename CODER>
class base_bitmap : public crpt<IMPL> {
protected:
    int _width = 0, _height = 0;
    microgl::coder::PixelCoder<P, CODER> _coder;
    buffer<P> _buffer;

public:

    static constexpr bool hasNativeAlphaChannel() { return CODER::alpha_bits()!=0; }
    static constexpr bool nativeAlphaChannelBits() { return hasNativeAlphaChannel() ? CODER::alpha_bits() : 8; }
    static constexpr int maxNativeAlphaChannelValue() { return (1u<<nativeAlphaChannelBits())-1; }

    base_bitmap(int w, int h) : base_bitmap(new uint8_t[sizeof(P) * w * h], w, h) {}
    base_bitmap(uint8_t *$pixels, int w, int h) : base_bitmap($pixels, w*h, w, h) {}
    base_bitmap(uint8_t *$pixels, int size, int w, int h) :
            _width{w}, _height{h}, _buffer(reinterpret_cast<P *>($pixels), size) {
    }
    ~base_bitmap() { _width =_height=0; }

    int width() const { return _width; }
    int height() const { return _height; }
    int size() const { return _buffer.size();}
    P * data() { return _buffer._data; }

    int locate(int x, int y) const { return y*this->_width + x; }
    P pixelAt(int x, int y) const { return this->pixelAt(y*this->_width + x); }
    P pixelAt(int index) const { return this->derived().pixelAt(index); }
    void writeAt(int x, int y, const P &value) { this->writeAt(y*this->_width + x, value); }
    void writeAt(int index, const P &value) { this->derived().writeAt(index, value); }
    void fill(const P &value) { this->derived().fill(value); }

    const microgl::coder::PixelCoder<P, CODER> &coder() const { return _coder; }

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
    }

    void writeColor(int x, int y, const microgl::color::color_t &color) {
        writeColor(y*_width + x, color);
    }

    template <typename number>
    void writeColor(int index, const microgl::color::intensity<number> &color) {
        P output;
        _coder.encode(color, output);
        writeAt(index, output);
    }

    template <typename number>
    void writeColor(int x, int y, const microgl::color::intensity<number> &color) {
        writeColor(y*_width + x, color);
    }

};