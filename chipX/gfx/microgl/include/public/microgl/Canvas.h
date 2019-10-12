#pragma once

#include <microgl/vec2.h>
#include <microgl/color.h>
#include <microgl/Functions.h>
#include <microgl/PorterDuff.h>
#include <microgl/BlendMode.h>
#include <microgl/PixelCoder.h>
#include <microgl/Bitmap.h>
#include <microgl/Fixed.h>
#include <microgl/Sampler.h>
#include <microgl/Curves.h>
#include <microgl/triangles.h>
#include <microgl/polygons.h>
#include <microgl/tesselation/BezierCurveDivider.h>
#include <microgl/dynamic_array.h>
#include <microgl/tesselation/EarClippingTriangulation.h>
#include <microgl/tesselation/FanTriangulation.h>

using namespace microgl::triangles;
//using namespace microgl;
using namespace microgl::polygons;

template<typename P, typename CODER>
class Canvas {
public:
    using index = unsigned int;
    using precision = unsigned char;
    using opacity = unsigned char;

    explicit Canvas(Bitmap<P, CODER> * $bmp);
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
                    float centerX, float centerY,
                    float radius, uint8_t opacity=255);

    // Triangles
    
    // main color
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangles(const color_f_t & color,
                       const vec2_32i *vertices,
                       const index *indices,
                       const boundary_info * boundary_buffer,
                       index size,
                       TrianglesIndices type,
                       uint8_t opacity,
                       uint8_t sub_pixel_precision);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangles(const color_f_t & color,
                       const vec2_f *vertices,
                       const index *indices,
                       const boundary_info * boundary_buffer,
                       index size,
                       TrianglesIndices type,
                       uint8_t opacity,
                       uint8_t requested_sub_pixel_precision);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTrianglesWireframe(const color_f_t & color,
                                const vec2_32i *vertices,
                                const index *indices,
                                index size,
                                TrianglesIndices type,
                                uint8_t opacity,
                                uint8_t sub_pixel_precision);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTrianglesWireframe(const color_f_t & color,
                                const vec2_f *vertices,
                                const index *indices,
                                index size,
                                TrianglesIndices type,
                                uint8_t opacity,
                                uint8_t sub_pixel_precision);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangleWireframe(const color_f_t &color,
                               const vec2_32i &p0,
                               const vec2_32i &p1,
                               const vec2_32i &p2,
                               uint8_t sub_pixel_precision);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangleWireframe(const color_f_t &color,
                               const vec2_f &p0,
                               const vec2_f &p1,
                               const vec2_f &p2,
                               uint8_t sub_pixel_precision);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangle(const color_f_t & color,
                      fixed_signed v0_x, fixed_signed v0_y,
                      fixed_signed v1_x, fixed_signed v1_y,
                      fixed_signed v2_x, fixed_signed v2_y,
                      uint8_t opacity,
                      uint8_t sub_pixel_precision,
                      bool aa_first_edge = true,
                      bool aa_second_edge = true,
                      bool aa_third_edge = true);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangleFast(const color_f_t & color,
                          fixed_signed v0_x, fixed_signed v0_y,
                          fixed_signed v1_x, fixed_signed v1_y,
                          fixed_signed v2_x, fixed_signed v2_y,
                          uint8_t opacity,
                          uint8_t sub_pixel_precision,
                          bool aa_first_edge = true,
                          bool aa_second_edge = true,
                          bool aa_third_edge = true);

    // float version
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangle(const color_f_t & color,
                      float v0_x, float v0_y,
                      float v1_x, float v1_y,
                      float v2_x, float v2_y,
                      uint8_t opacity = 255,
                      bool aa_first_edge = true,
                      bool aa_second_edge = true,
                      bool aa_third_edge = true);

    // main uv

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, bool perspective_correct=false,
            typename Sampler=sampler::NearestNeighbor,
            typename P2, typename CODER2>
    void drawTriangle(const Bitmap<P2, CODER2> &bmp,
                      fixed_signed v0_x, fixed_signed v0_y, fixed_signed u0, fixed_signed v0, fixed_signed q0,
                      fixed_signed v1_x, fixed_signed v1_y, fixed_signed u1, fixed_signed v1, fixed_signed q1,
                      fixed_signed v2_x, fixed_signed v2_y, fixed_signed u2, fixed_signed v2, fixed_signed q2,
                      opacity opacity, precision sub_pixel_precision, precision uv_precision,
                      bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false,
            typename Sampler=sampler::NearestNeighbor,
            typename P2, typename CODER2>
    void drawTriangle(const Bitmap<P2, CODER2> &bmp,
                      float v0_x, float v0_y, float u0, float v0,
                      float v1_x, float v1_y, float u1, float v1,
                      float v2_x, float v2_y, float u2, float v2,
                      opacity opacity = 255,
                      bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

    // Quadrilaterals
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename Sampler=sampler::NearestNeighbor,
            typename P2, typename CODER2>
    void drawQuadrilateral(const Bitmap<P2, CODER2> &bmp,
                           fixed_signed v0_x, fixed_signed v0_y, fixed_signed u0, fixed_signed v0,
                           fixed_signed v1_x, fixed_signed v1_y, fixed_signed u1, fixed_signed v1,
                           fixed_signed v2_x, fixed_signed v2_y, fixed_signed u2, fixed_signed v2,
                           fixed_signed v3_x, fixed_signed v3_y, fixed_signed u3, fixed_signed v3,
                           opacity opacity, precision sub_pixel_precision, precision uv_precision);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename Sampler=sampler::NearestNeighbor,
            typename P2, typename CODER2>
    void drawQuadrilateral(const Bitmap<P2, CODER2> &bmp,
                           float v0_x, float v0_y, float u0, float v0,
                           float v1_x, float v1_y, float u1, float v1,
                           float v2_x, float v2_y, float u2, float v2,
                           float v3_x, float v3_y, float u3, float v3,
                           opacity opacity = 255);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawQuadrilateral(const color_f_t & color,
                           int v0_x, int v0_y,
                           int v1_x, int v1_y,
                           int v2_x, int v2_y,
                           int v3_x, int v3_y,
                           precision sub_pixel_precision,
                           opacity opacity = 255);


    // QUADS

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque>
    void drawQuad(const color_f_t &color,
                  int left, int top,
                  int right, int bottom,
                  precision sub_pixel_precision,
                  opacity opacity = 255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque>
    void drawQuad(const color_f_t &color,
                  float left, float top,
                  float right, float bottom,
                  opacity opacity = 255);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            typename Sampler=sampler::NearestNeighbor,
            typename P2, typename CODER2>
    void drawQuad(const Bitmap<P2, CODER2> &bmp,
                  int left, int top,
                  int right, int bottom,
                  int u0, int v0,
                  int u1, int v1,
                  precision sub_pixel_precision, precision uv_precision,
                  opacity opacity = 255);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            typename Sampler=sampler::NearestNeighbor,
            typename P2, typename CODER2>
    void drawQuad(const Bitmap<P2, CODER2> &bmp,
                  float left, float top,
                  float right, float bottom,
                  float u0=0.0f, float v0=0.0f,
                  float u1=1.0f, float v1=1.0f,
                  opacity opacity = 255);

    // polygons

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawPolygon(vec2_32i * points,
                     index size,
                     precision precision,
                     opacity opacity,
                     polygons::hints hint = polygons::hints::SIMPLE);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawPolygon(vec2_f * points,
                     index size,
                     opacity opacity,
                     polygons::hints hint = polygons::hints::SIMPLE);

    // paths
    void drawLine(const color_f_t & color,
                  float x0, float y0, float x1, float y1);

    void drawLine(const color_f_t & color,
                  int x0, int y0,
                  int x1, int y1,
                  uint8_t bits = 0);

    void drawLine(const color_f_t & color,
                  const vec2_32i &p0,
                  const vec2_32i &p1,
                  uint8_t bits = 0);

    void drawQuadraticBezierPath(color_f_t & color, vec2_32i *points,
                                 unsigned int size = 3,
                                 uint8_t sub_pixel_bits = 4,
                                 tessellation::BezierCurveDivider::CurveDivisionAlgorithm algorithm
                                 = tessellation::BezierCurveDivider::CurveDivisionAlgorithm::Uniform_16);

    void drawQuadraticBezierPath(color_f_t & color, vec2_f *points,
                                 unsigned int size = 3,
                                 tessellation::BezierCurveDivider::CurveDivisionAlgorithm algorithm =
                                         tessellation::BezierCurveDivider::CurveDivisionAlgorithm::Uniform_16);

    void drawCubicBezierPath(color_f_t & color, vec2_32i *points,
                             unsigned int size = 4,
                             uint8_t sub_pixel_bits = 4,
                             tessellation::BezierCurveDivider::CurveDivisionAlgorithm algorithm =
                                     tessellation::BezierCurveDivider::CurveDivisionAlgorithm::Uniform_16);

    void drawCubicBezierPath(color_f_t & color, vec2_f *points,
                             unsigned int size = 4,
                             tessellation::BezierCurveDivider::CurveDivisionAlgorithm algorithm =
                                     tessellation::BezierCurveDivider::CurveDivisionAlgorithm::Uniform_16);

    void drawLinePath(color_f_t & color,
                      vec2_32i *points,
                      precision precision,
                      unsigned int size = 4,
                      bool closed_path = false);

    void drawLinePath(color_f_t & color,
                      vec2_f *points,
                      unsigned int size = 4,
                      bool closed_path = false);

private:
    int _width = 0, _height = 0;
    Bitmap<P, CODER> * _bitmap_canvas = nullptr;
    bool _flag_antiAlias = true;

    // compositing
    bool _flag_hasNativeAlphaChannel = false;
    uint8_t _alpha_bits_for_compositing = 8;
    unsigned int _max_alpha_value = 255;

};

#include "../../src/Canvas.tpp"
