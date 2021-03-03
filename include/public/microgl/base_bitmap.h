#pragma once

#include <microgl/buffer.h>
#include <microgl/crpt.h>
#include <microgl/color.h>
#include <cstdint>

template <typename impl, typename pixel_coder_, typename buffer_element_type=typename pixel_coder_::pixel>
class base_bitmap : public crpt<impl> {
public:
    using pixel_coder=pixel_coder_;
    using pixel=typename pixel_coder::pixel;
    using rgba=typename pixel_coder::rgba;

protected:
    int _width = 0, _height = 0;
    pixel_coder _coder;
    buffer<buffer_element_type> _buffer;
    // todo:: add a sub window feature, only involves translating x and y coords or a fixed index ?

public:
    static constexpr bool hasNativeAlphaChannel() { return pixel_coder::rgba::a != 0; }
    static constexpr bool nativeAlphaChannelBits() { return hasNativeAlphaChannel() ? pixel_coder::rgba::a : 8; }
    static constexpr int maxNativeAlphaChannelValue() { return (1u<<nativeAlphaChannelBits())-1; }

    base_bitmap(int w, int h) : base_bitmap(new uint8_t[sizeof(buffer_element_type) * w * h], w, h) {}
    base_bitmap(uint8_t *$pixels, int w, int h) : base_bitmap(reinterpret_cast<buffer_element_type *>($pixels), w*h, w, h) {}
    base_bitmap(buffer_element_type *$pixels, int size, int w, int h) :
            _width{w}, _height{h}, _coder{}, _buffer($pixels, size) {
    }
    ~base_bitmap() { _width =_height=0; }

    int width() const { return _width; }
    int height() const { return _height; }
    int size() const { return _buffer.size();}
    pixel * data() { return _buffer._data; }

    int locate(int x, int y) const { return y*this->_width + x; }
    pixel pixelAt(int x, int y) const { return this->pixelAt(y*this->_width + x); }
    pixel pixelAt(int index) const { return this->derived().pixelAt(index); }
    void writeAt(int x, int y, const pixel &value) { this->writeAt(y*this->_width + x, value); }
    void writeAt(int index, const pixel &value) { this->derived().writeAt(index, value); }
    void fill(const pixel &value) { this->derived().fill(value); }
    // replace with just CODER ?
    const pixel_coder &coder() const { return _coder; }

    void decode(int x, int y, microgl::color::color_t &output) const{
        _coder.decode(pixelAt(x, y), output);
    }

    void decode(int index, microgl::color::color_t &output) const{
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
        pixel output;
        _coder.encode(color, output);
        writeAt(index, output);
    }

    void writeColor(int x, int y, const microgl::color::color_t &color) {
        writeColor(y*_width + x, color);
    }

    template <typename number>
    void writeColor(int index, const microgl::color::intensity<number> &color) {
        pixel output;
        _coder.encode(color, output);
        writeAt(index, output);
    }

    template <typename number>
    void writeColor(int x, int y, const microgl::color::intensity<number> &color) {
        writeColor(y*_width + x, color);
    }

};