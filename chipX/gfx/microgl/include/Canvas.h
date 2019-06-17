//
// Created by Tomer Shalev on 2019-06-16.
//

#pragma once

#include "Types.h"
#include "FrameBuffer.h"
#include "PixelFormat.h"
#include "Functions.h"
#include "PorterDuff.h"
#include "BlendMode.h"

template<typename T>
class Canvas {
public:
    Canvas(FrameBuffer<T> * frameBuffer);
    Canvas(int width, int height, PixelFormat format);
    int width();
    int height();
    PixelFormat pixelFormat();
    unsigned int sizeofPixel();
    T* pixels();
    T getPixel(int x, int y);
    color_f_t getPixelColor(int x, int y);

    void setBlendMode(const BlendMode & mode);
    BlendMode & getBlendMode();
    void setPorterDuffMode(const PorterDuff & mode);
    PorterDuff & getPorterDuffMode();
    bool hasAlphaChannel();

    void clear(const color_f_t &color);
    void drawPixel(const color_f_t &val, int x, int y);
    void drawPixel(const T &val, int x, int y);
    void drawQuad(const color_f_t &color, int left, int top, int w, int h);
    void drawGradient(const color_f_t &startColor, const color_f_t &endColor, int left, int top, int w, int h);
    void drawCircle(const color_f_t & color, int centerX, int centerY, int radius);

private:
    FrameBuffer<T> * _fb = nullptr;
    BlendMode _blend_mode = BlendMode::Normal;
    PorterDuff _porter_duff_mode = PorterDuff::SourceOver;
    bool _flag_hasAlphaChannel = false;
};

typedef Canvas<uint8_t> Canvas8Bit;
typedef Canvas<uint16_t> Canvas16Bit;
typedef Canvas<uint32_t> Canvas32Bit;


#include "../src/Canvas.tpp"


