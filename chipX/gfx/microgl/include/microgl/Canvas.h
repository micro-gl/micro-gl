#pragma once

#include "Types.h"
#include "FrameBuffer.h"
#include "PixelFormat.h"
#include "Functions.h"
#include "PorterDuff.h"
#include "BlendMode.h"
#include "PixelCoder.h"
#include "Bitmap.h"
#include "BlendMode.h"
#include "PorterDuff.h"
#include "Fixed.h"

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
    void getPixelColor(int index, color_t & output);
    void getPixelColor(int x, int y, color_t & output);
    void getPixelColor(int x, int y, color_f_t & output);
    void getPixelColor(int index, color_f_t & output);

    PixelCoder<P, CODER> * coder();
    Bitmap<P, CODER> * bitmapCanvas();

    bool hasNativeAlphaChannel();
    bool hasAntialiasing();
    void setAntialiasing(bool value);

    void clear(const color_f_t &color);

    // float blenders
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::SourceOverOnOpaque>
    void blendColor(const color_f_t &val, int x, int y, float opacity=1.0f);
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::SourceOverOnOpaque>
    void blendColor(const color_f_t &val, int index, float opacity=1.0f);

    // integer blenders
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::SourceOverOnOpaque>
    void blendColor(const color_t &val, int x, int y, uint8_t opacity);
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::SourceOverOnOpaque>
    void blendColor(const color_t &val, int index, uint8_t opacity);

    void drawPixel(const P &val, int x, int y);
    void drawPixel(const P &val, int index);

    void drawLine(const color_f_t & color,
                  int x0, int y0, int x1, int y1);

    void drawGradient(const color_f_t &startColor,
                      const color_f_t &endColor,
                      int left, int top, int w, int h);

    // circles
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawCircle(const color_f_t & color,
                    fixed_signed centerX, fixed_signed centerY,
                    fixed_signed radius, uint8_t sub_pixel_precision,
                    uint8_t opacity=255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawCircle(const color_f_t & color,
                    int centerX, int centerY,
                    int radius, uint8_t opacity=255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawCircle(const color_f_t & color,
                    const float centerX, const float centerY,
                    const float radius, uint8_t opacity=255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque>
    void drawCircleFPU(const color_f_t & color,
                    int centerX, int centerY,
                    int radius, float opacity=1.0f);

    // Triangles
    
    // main color
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangle(const color_f_t & color,
                      const fixed_signed x0, const fixed_signed y0,
                      const fixed_signed x1, const fixed_signed y1,
                      const fixed_signed x2, const fixed_signed y2,
                      const uint8_t opacity, const uint8_t sub_pixel_precision);
    // integer version
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangle(const color_f_t & color,
                      const int x0, const int y0,
                      const int x1, const int y1,
                      const int x2, const int y2,
                      const uint8_t opacity=255);
    // float version
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangle(const color_f_t & color,
                     const float x0, const float y0,
                     const float x1, const float y1,
                     const float x2, const float y2,
                     const uint8_t opacity = 255);

    // main uv
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, bool perspective_correct=true,
            typename P2, typename CODER2>
    void drawTriangle(const Bitmap<P2, CODER2> &bmp,
                      const fixed_signed v0_x, const fixed_signed v0_y, fixed_signed u0, fixed_signed v0, fixed_signed q0,
                      const fixed_signed v1_x, const fixed_signed v1_y, fixed_signed u1, fixed_signed v1, fixed_signed q1,
                      const fixed_signed v2_x, const fixed_signed v2_y, fixed_signed u2, fixed_signed v2, fixed_signed q2,
                      const uint8_t opacity, const uint8_t sub_pixel_precision, const uint8_t uv_precision);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false,
            typename P2, typename CODER2>
    void drawTriangle(const Bitmap<P2, CODER2> &bmp,
                      const float v0_x, const float v0_y, float u0, float v0,
                      const float v1_x, const float v1_y, float u1, float v1,
                      const float v2_x, const float v2_y, float u2, float v2,
                      const uint8_t opacity = 255);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false,
            typename P2, typename CODER2>
    void drawTriangle(const Bitmap<P2, CODER2> &bmp,
                      const int v0_x, const int v0_y, float u0, float v0,
                      const int v1_x, const int v1_y, float u1, float v1,
                      const int v2_x, const int v2_y, float u2, float v2,
                      const uint8_t opacity = 255);



    // Quadrilaterals
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename P2, typename CODER2>
    void drawQuadrilateral(const Bitmap<P2, CODER2> &bmp,
                           const fixed_signed x0, const fixed_signed y0, fixed_signed u0, fixed_signed v0,
                           const fixed_signed x1, const fixed_signed y1, fixed_signed u1, fixed_signed v1,
                           const fixed_signed x2, const fixed_signed y2, fixed_signed u2, fixed_signed v2,
                           const fixed_signed x3, const fixed_signed y3, fixed_signed u3, fixed_signed v3,
                           const uint8_t opacity, const uint8_t sub_pixel_precision, const uint8_t uv_precision);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename P2, typename CODER2>
    void drawQuadrilateral(const Bitmap<P2, CODER2> &bmp,
                           const float x0, const float y0, float u0, float v0,
                           const float x1, const float y1, float u1, float v1,
                           const float x2, const float y2, float u2, float v2,
                           const float x3, const float y3, float u3, float v3,
                           const uint8_t opacity = 255);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawQuadrilateral(const color_f_t & color,
                           const int x0, const int y0,
                           const int x1, const int y1,
                           const int x2, const int y2,
                           const int x3, const int y3,
                           const uint8_t opacity = 255);


    // QUADS

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque>
    void drawQuad(const color_f_t &color,
                  const fixed_signed left, const fixed_signed top,
                  const fixed_signed right, const fixed_signed bottom,
                  const uint8_t sub_pixel_precision,
                  const uint8_t opacity = 255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque>
    void drawQuad(const color_f_t &color,
                  const float left, const float top,
                  const float right, const float bottom,
                  const uint8_t opacity = 255);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            typename P2, typename CODER2>
    void drawQuad(const Bitmap<P2, CODER2> &bmp,
                  const fixed_signed left, const fixed_signed top,
                  const fixed_signed right, const fixed_signed bottom,
                  const fixed_signed u0, const fixed_signed v0,
                  const fixed_signed u1, const fixed_signed v1,
                  const uint8_t sub_pixel_precision, const uint8_t uv_precision,
                  const uint8_t opacity = 255);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            typename P2, typename CODER2>
    void drawQuad(const Bitmap<P2, CODER2> &bmp,
                  const float left, const float top,
                  const float right, const float bottom,
                  const float u0=0.0f, const float v0=0.0f,
                  const float u1=1.0f, const float v1=1.0f,
                  const uint8_t opacity = 255);


    // paths
    void drawQuadraticBezierPath(color_f_t & color, vec2_32i *points,
                                 unsigned int size = 3, unsigned int resolution_bits = 5);
    void drawCubicBezierPath(color_f_t & color, vec2_32i *points,
                             unsigned int size = 4, unsigned int resolution_bits = 5);
    void drawLinePath(color_f_t & color, vec2_32i *points,
                             unsigned int size = 4);


private:
    int _width = 0, _height = 0;
    Bitmap<P, CODER> * _bitmap_canvas = nullptr;
    bool _flag_antiAlias = true;

    // compositing
    bool _flag_hasNativeAlphaChannel = false;
    uint8_t _alpha_bits_for_compositing = 8;
    unsigned int _max_alpha_value = 255;

};

#include "../src/Canvas.tpp"


