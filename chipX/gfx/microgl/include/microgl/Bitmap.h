//
// Created by Tomer Shalev on 2019-06-22.
//

#pragma once

#include "FrameBuffer.h"
#include "PixelCoder.h"

template <typename P>
class Bitmap : public FrameBuffer<P> {
public:
    Bitmap(int w, int h, PixelCoder<P> * $coder);
    Bitmap(P* $pixels, int w, int h, PixelCoder<P> * $coder);
    Bitmap(uint8_t* $pixels, int w, int h, PixelCoder<P> * $coder);
    ~Bitmap();

    P pixelAt(int x, int y);
    P pixelAt(int index);
    color_t decodePixelAt(int x, int y);
    color_t decodePixelAt(int index);
    color_f_t decodeNormalizedPixelAt(int x, int y);
    color_f_t decodeNormalizedPixelAt(int index);
    int width();
    int height();
    PixelFormat & format();
    PixelCoder<P> * coder();

protected:
    int _width = 0, _height = 0;
    PixelFormat _format;
    PixelCoder<P> * _coder;

};

typedef Bitmap<vec3<uint8_t>> Bitmap24bitU8;
typedef Bitmap<uint32_t> Bitmap32bitPacked;
typedef Bitmap<uint16_t> Bitmap16BitPacked;
typedef Bitmap<uint8_t> Bitmap8Bit;

#include "../src/Bitmap.tpp"