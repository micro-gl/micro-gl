//
// Created by Tomer Shalev on 2019-06-22.
//

#pragma once

#include "FrameBuffer.h"
#include "PixelCoder.h"

template <typename P, typename CODER>
class Bitmap : public FrameBuffer<P> {
public:
    Bitmap(int w, int h, PixelCoder<P, CODER> * $coder);
    Bitmap(P* $pixels, int w, int h, PixelCoder<P, CODER> * $coder);
    Bitmap(uint8_t* $pixels, int w, int h, PixelCoder<P, CODER> * $coder);
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
    PixelCoder<P, CODER> * coder();

protected:
    int _width = 0, _height = 0;
    PixelFormat _format;
    PixelCoder<P, CODER> * _coder;

};

//
//typedef Bitmap<vec3<uint8_t>, CODER> Bitmap24bitU8<CODER>;
//typedef Bitmap<uint32_t, CODER> Bitmap32bitPacked;
//typedef Bitmap<uint16_t, CODER> Bitmap16BitPacked;
//typedef Bitmap<uint8_t, CODER> Bitmap8Bit;

#include "../src/Bitmap.tpp"