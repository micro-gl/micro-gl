#pragma once

#include <microgl/FrameBuffer.h>
#include <microgl/PixelCoder.h>

template <typename P, typename CODER>
class Bitmap : public FrameBuffer<P> {
public:
    template <typename P2, typename CODER2>
    Bitmap<P2, CODER2> * convertToBitmap();

    template <typename P2, typename CODER2>
    void copyToBitmap(Bitmap<P2, CODER2> & bmp);

    Bitmap(int w, int h);
    Bitmap(P* $pixels, int w, int h);
    Bitmap(uint8_t* $pixels, int w, int h);
    ~Bitmap();

    P pixelAt(int x, int y) const;
    P pixelAt(int index) const;
    // decoders
    void decode(int x, int y, color_t &output) const;
    void decode(int index, color_t &output) const;
    template <typename number>
    void decode(int x, int y, intensity<number> &output) const;
    template <typename number>
    void decode(int index, intensity<number> &output) const;
    // encoders
    void writeColor(int index, const color_t & color);
    void writeColor(int x, int y, const color_t & color);
    template <typename number>
    void writeColor(int index, const intensity<number> & color);
    template <typename number>
    void writeColor(int x, int y, const intensity<number> & color);
    int width() const;
    int height() const;
    const microgl::coder::PixelCoder<P, CODER> & coder() const;

protected:
    int _width = 0, _height = 0;
    microgl::coder::PixelCoder<P, CODER> _coder;
};

#include "../../src/Bitmap.tpp"