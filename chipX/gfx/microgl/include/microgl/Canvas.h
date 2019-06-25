#pragma once

#include "Types.h"
#include "FrameBuffer.h"
#include "PixelFormat.h"
#include "Functions.h"
#include "PorterDuff.h"
#include "BlendMode.h"
#include "PixelCoder.h"
#include "Bitmap.h"

template<typename P, typename CODER>
class Canvas {
public:

    Canvas(Bitmap<P, CODER> * $bmp);
    Canvas(int width, int height, PixelCoder<P, CODER> * $coder);
    int width();
    int height();
    PixelFormat pixelFormat();
    unsigned int sizeofPixel();

    P* pixels();
    P &getPixel(int x, int y);
    P &getPixel(int index);
    color_f_t getPixelColor(int index);
    color_f_t getPixelColor(int x, int y);
    void getPixelColor(int x, int y, color_f_t & output);
    void getPixelColor(int index, color_f_t & output);

    PixelCoder<P, CODER> * coder();
    Bitmap<P, CODER> * bitmapCanvas();

    void setBlendMode(const BlendMode & mode);
    BlendMode & getBlendMode();
    void setPorterDuffMode(const PorterDuff & mode);
    PorterDuff & getPorterDuffMode();
    bool hasAlphaChannel();
    bool hasAntialiasing();
    void setAntialiasing(bool value);

    void clear(const color_f_t &color);
    void blendColor(const color_f_t &val, int x, int y);
    void blendColor(const color_f_t &val, int index);
    void drawPixel(const P &val, int x, int y);
    void drawPixel(const P &val, int index);
    void drawQuad(const color_f_t &color, int left, int top, int w, int h);
    void drawGradient(const color_f_t &startColor, const color_f_t &endColor, int left, int top, int w, int h);
    void drawCircle(const color_f_t & color, int centerX, int centerY, int radius);
    void drawTriangle(const color_f_t & color, int x0, int y0, int x1, int y1, int x2, int y2);
    void drawTriangle2(Bitmap<P, CODER> &bmp,
                       int v0_x, int v0_y, float u0, float v0,
                       int v1_x, int v1_y, float u1, float v1,
                       int v2_x, int v2_y, float u2, float v2);
    void drawQuad2(Bitmap<P, CODER> &bmp, int left, int top, int w, int h);

//    void drawTriangle(FrameBuffer<T> * bitmap, int x0, int y0, int x1, int y1, int x2, int y2, );

private:
    int _width = 0, _height = 0;
    Bitmap<P, CODER> * _bitmap_canvas = nullptr;
    BlendMode _blend_mode = BlendMode::Normal;
    PorterDuff _porter_duff_mode = PorterDuff::SourceOver;
    bool _flag_hasAlphaChannel = false;
    bool _flag_antiAlias = true;
};


//typedef Canvas<uint8_t> Canvas8Bit;
//typedef Canvas<uint16_t> Canvas16Bit;
//typedef Canvas<uint32_t> Canvas32Bit;
//typedef Canvas<vec3<uint8_t>> Canvas24BitU8;

#include "../src/Canvas.tpp"


