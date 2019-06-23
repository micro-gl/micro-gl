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
    int width();
    int height();
    PixelFormat & format();
    PixelCoder<P> * coder();

protected:
    int _width = 0, _height = 0;
    PixelFormat _format;
    PixelCoder<P> * _coder;

};

#include "../src/Bitmap.tpp"