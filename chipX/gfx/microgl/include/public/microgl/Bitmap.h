//
// Created by Tomer Shalev on 2019-06-22.
//

#pragma once

#include "FrameBuffer.h"
#include "PixelCoder.h"

template <typename P, typename CODER>
class Bitmap : public FrameBuffer<P> {
public:
    template <typename P2, typename CODER2>
    Bitmap<P2, CODER2> * convertToBitmap();

    template <typename P2, typename CODER2>
    void copyToBitmap(Bitmap<P2, CODER2> & bmp);

    Bitmap(int w, int h, PixelCoder<P, CODER> * $coder);
    Bitmap(P* $pixels, int w, int h, PixelCoder<P, CODER> * $coder);
    Bitmap(uint8_t* $pixels, int w, int h, PixelCoder<P, CODER> * $coder);
    ~Bitmap();

    P pixelAt(int x, int y) const;
    P pixelAt(int index) const;

    // decoders
    void decode(int x, int y, color_t &output) const;
    void decode(int index, color_t &output) const;
    void decode(int x, int y, color_f_t &output) const;
    void decode(int index, color_f_t &output) const;
    // encoders
//    void encodeColorToPixel(color_t &color, P & output);
//    void encodeColorToPixel(int index, color_t &output);
//    void encodeNormalizedColorToPixel(int x, int y, color_f_t &output);
//    void encodeNormalizedColorToPixel(int index, color_f_t &output);

    void writeColor(int index, const color_t & color);
    void writeColor(int x, int y, const color_t & color);
    void writeColor(int index, const color_f_t & color);
    void writeColor(int x, int y, const color_f_t & color);

    int width() const;
    int height() const;
    PixelFormat format();
    PixelCoder<P, CODER> * coder();

protected:
    int _width = 0, _height = 0;
    PixelFormat _format;
    PixelCoder<P, CODER> * _coder;

};

#include "../../src/Bitmap.tpp"