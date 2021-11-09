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

#include "buffer.h"
#include "../color.h"
#include "../pixel_coders/pixel_coder.h"
#include "../traits.h"
#include "../stdint.h"

/**
 * a base bitmap type, use it with crpt design pattern for extension.
 * Bitmap is a general type that :
 * 1. references an array of pixels
 * 2. has a pixel coder
 * 3. has interface to read/write pixels
 *
 * @tparam impl implementation type of derived class
 * @tparam allocator_type the allocator type
 * @tparam pixel_coder_ the pixel coder type of the bitmap
 * @tparam buffer_element_type the type of the elements stored in the pixel array
 */
template <typename impl, class Allocator, typename pixel_coder_,
          typename buffer_element_type=typename pixel_coder_::pixel>
class base_bitmap : public microgl::traits::crpt<impl> {
public:
    using pixel_coder=pixel_coder_;
    using pixel=typename pixel_coder::pixel;
    using rgba=typename pixel_coder::rgba;
    using allocator_type = Allocator;

protected:

    int _width = 0, _height = 0;
    pixel_coder _coder;
    buffer<buffer_element_type, allocator_type> _buffer;
    // todo:: add a sub window feature, only involves translating x and y coords or a fixed index ?

public:
    static constexpr bool hasNativeAlphaChannel() { return pixel_coder::rgba::a != 0; }
    static constexpr bool nativeAlphaChannelBits() { return hasNativeAlphaChannel() ? pixel_coder::rgba::a : 0; }
    static constexpr int maxNativeAlphaChannelValue() { return (1u<<nativeAlphaChannelBits())-1; }

    base_bitmap(int w, int h, const allocator_type & allocator) :
            _width{w}, _height{h}, _coder{}, _buffer(w*h, allocator) {}
    base_bitmap(void *$pixels, int w, int h, const allocator_type & allocator) :
            _width{w}, _height{h}, _coder{},
            _buffer(reinterpret_cast<buffer_element_type *>($pixels), w*h, allocator) {}//base_bitmap($pixels, w*h, w, h, false) {}
    base_bitmap(void *$pixels, int size, int w, int h,
                const allocator_type & allocator) :
            _width{w}, _height{h}, _coder{},
            _buffer(reinterpret_cast<buffer_element_type *>($pixels), size, allocator) {
    }


    base_bitmap(const base_bitmap & bmp) : _buffer{bmp._buffer}, _width{bmp.width()},
                            _height{bmp.height()} {
    }
    base_bitmap(base_bitmap && bmp) noexcept : _buffer{microgl::traits::move(bmp._buffer)},
                    _width{bmp.width()}, _height{bmp.height()} {
    }

    base_bitmap & operator=(const base_bitmap & bmp) {
        _width=bmp.width(); _height=bmp.height();
        _buffer = bmp._buffer;
        return *this;
    }
    base_bitmap & operator=(base_bitmap && bmp) noexcept {
        _width=bmp.width(); _height=bmp.height();
        _buffer = microgl::traits::move(bmp._buffer);
        return *this;
    }
    virtual ~base_bitmap() = default;

    // does the underlying pixel buffer own the data ?
    bool isOwner() const { return _buffer.owner; }
    int width() const { return _width; }
    int height() const { return _height; }
    int size() const { return _buffer.size();}
    const pixel * data() const { return _buffer.data(); }
    pixel * data() { return _buffer.data(); }

    int locate(int x, int y) const { return y*this->_width + x; }
    pixel pixelAt(int x, int y) const { return this->pixelAt(y*this->_width + x); }
    pixel pixelAt(int index) const { return this->derived().pixelAt(index); }
    void writeAt(int x, int y, const pixel &value) { this->writeAt(y*this->_width + x, value); }
    void writeAt(int index, const pixel &value) { this->derived().writeAt(index, value); }
    void fill(const pixel &value) { this->derived().fill(value); }

    const pixel_coder &coder() const { return _coder; }

    void decode(int x, int y, microgl::color_t &output) const{
        _coder.decode(pixelAt(x, y), output);
    }

    void decode(int index, microgl::color_t &output) const{
        _coder.decode(pixelAt(index), output);
    }

    template <typename number>
    void decode(int x, int y, microgl::intensity<number> &output) const {
        microgl::coder::decode<number, pixel_coder>(pixelAt(x, y), output, coder());
    }

    template <typename number>
    void decode(int index, microgl::intensity<number> &output) const {
        microgl::coder::decode<number, pixel_coder>(pixelAt(index), output, coder());
    }

    void writeColor(int index, const microgl::color_t &color) {
        pixel output;
        _coder.encode(color, output);
        writeAt(index, output);
    }

    void writeColor(int x, int y, const microgl::color_t &color) {
        writeColor(y*_width + x, color);
    }

    template <typename number>
    void writeColor(int index, const microgl::intensity<number> &color) {
        pixel output;
        microgl::coder::encode<number, pixel_coder>(color, output, coder());
        writeAt(index, output);
    }

    template <typename number>
    void writeColor(int x, int y, const microgl::intensity<number> &color) {
        writeColor<number>(y*_width + x, color);
    }

};