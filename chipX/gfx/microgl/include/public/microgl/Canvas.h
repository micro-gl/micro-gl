#pragma once

#include <microgl/vec2.h>
#include <microgl/color.h>
#include <microgl/micro_gl_traits.h>
#include <microgl/porter_duff/SourceOverOnOpaque.h>
#include <microgl/porter_duff/DestinationIn.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/Bitmap.h>
#include <microgl/shader.h>
#include <microgl/sampler.h>
#include <microgl/triangles.h>
#include <microgl/polygons.h>
#include <microgl/masks.h>
#include <microgl/math.h>
#include <microgl/tesselation/curve_divider.h>
#include <microgl/tesselation/ear_clipping_triangulation.h>
#include <microgl/tesselation/fan_triangulation.h>
#include <microgl/tesselation/bezier_patch_tesselator.h>
#include <microgl/cohen_sutherland_clipper.h>
#include <microgl/homo_triangle_clipper.h>
#include <microgl/uv_map.h>
#include <microgl/functions/minmax.h>
#include <microgl/functions/clamp.h>
#include <microgl/functions/swap.h>
#include <microgl/functions/orient2d.h>

using namespace microgl::triangles;
using namespace microgl::polygons;
using namespace microgl::shading;

template<typename P, typename CODER>
class Canvas {
private:
    using index = unsigned int;
    using precision = unsigned char;
    using opacity_t = unsigned char;
    using l64= long long;
    using canvas= Canvas<P, CODER>;

    int _width = 0, _height = 0;
    Bitmap<P, CODER> * _bitmap_canvas = nullptr;
    // compositing
    bool _flag_hasNativeAlphaChannel = false;
    uint8_t _alpha_bits_for_compositing = 8;
    unsigned int _max_alpha_value = 255;

public:
    explicit Canvas(Bitmap<P, CODER> * $bmp);
    Canvas(int width, int height);
    int width() const;
    int height() const;
    unsigned int sizeofPixel() const;
    P* pixels() const;
    P &getPixel(int x, int y) const ;
    P &getPixel(int index) const ;
    void getPixelColor(int index, color_t & output) const;
    void getPixelColor(int x, int y, color_t & output) const;
    void getPixelColor(int x, int y, color_f_t & output) const;
    void getPixelColor(int index, color_f_t & output) const;

    const coder::PixelCoder<P, CODER> & coder() const;
    Bitmap<P, CODER> * bitmapCanvas() const;
    bool hasNativeAlphaChannel() const;
    void clear(const color_f_t &color);
    void clear(const color_t &color);

//    // float blenders
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::SourceOverOnOpaque>
    void blendColor(const color_f_t &val, int x, int y, float opacity=1.0f);

    // integer blenders
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::SourceOverOnOpaque>
    void blendColor(const color_t &val, int x, int y, opacity_t opacity);
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::SourceOverOnOpaque>
    inline void blendColor(const color_t &val, int index, opacity_t opacity);

    void drawPixel(const P &val, int x, int y);
    void drawPixel(const P &val, int index);

    // circles

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number, typename S1, typename S2>
    void drawCircle(const sampling::sampler<S1> & sampler_fill,
                    const sampling::sampler<S2> & sampler_stroke,
                    number centerX, number centerY,
                    number radius, number stroke_size, opacity_t opacity=255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number, typename S1, typename S2>
    void drawRoundedQuad(const sampling::sampler<S1> & sampler_fill,
                         const sampling::sampler<S2> & sampler_stroke,
                         number left, number top,
                         number right, number bottom,
                         number radius, number stroke_size,
                         opacity_t opacity=255);

private:
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename S1, typename S2>
    void drawRoundedQuad(const sampling::sampler<S1> & sampler_fill,
                         const sampling::sampler<S2> & sampler_stroke,
                         int left, int top,
                         int right, int bottom,
                         int radius, int stroke_size,
                         precision sub_pixel_precision, opacity_t opacity=255);
public:

    // Triangle batches
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number=float>
    void drawTriangles(const color_f_t & color,
                       const vec2<number> *vertices,
                       const index *indices,
                       const boundary_info * boundary_buffer,
                       index size,
                       enum indices type,
                       opacity_t opacity);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=float, typename S>
    void drawTriangles(const sampling::sampler<S> & sampler,
                       const vec2<number1> *vertices,
                       const vec2<number2> *uvs,
                       const index *indices,
                       const boundary_info * boundary_buffer,
                       index size,
                       enum indices type,
                       opacity_t opacity);

    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias, bool perspective_correct, bool depth_buffer_flag=false,
            typename impl, typename vertex_attr, typename varying, typename number>
    void drawTriangles(shader_base<impl, vertex_attr, varying, number> &shader,
                       const vertex_attr *vertex_buffer,
                       const index *indices,
                       const boundary_info * boundary_buffer,
                       index size,
                       enum indices type,
                       const triangles::face_culling & culling= triangles::face_culling::none,
                        long long * depth_buffer=nullptr,
                       opacity_t opacity=255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number=float>
    void drawTrianglesWireframe(const color_f_t & color,
                                const vec2<number> *vertices,
                                const index *indices,
                                index size,
                                enum indices type,
                                opacity_t opacity);

    // single triangles, includes shader based and one very fast fixed pipeline for color and textures
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number=float>
    void drawTriangleWireframe(const color_f_t &color,
                               const vec2<number> &p0,
                               const vec2<number> &p1,
                               const vec2<number> &p2);

private:

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false>
    void drawTriangle(const color_f_t & color,
                      int v0_x, int v0_y,
                      int v1_x, int v1_y,
                      int v2_x, int v2_y,
                      opacity_t opacity,
                      precision sub_pixel_precision,
                      bool aa_first_edge = true,
                      bool aa_second_edge = true,
                      bool aa_third_edge = true);

public:

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number>
    void drawTriangle(const color_f_t & color,
                      number v0_x, number v0_y,
                      number v1_x, number v1_y,
                      number v2_x, number v2_y,
                      opacity_t opacity = 255,
                      bool aa_first_edge = true,
                      bool aa_second_edge = true,
                      bool aa_third_edge = true);

private:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, bool perspective_correct=false,
            typename S>
    void drawTriangle(const sampling::sampler<S> &sample,
                      int v0_x, int v0_y, int u0, int v0, int q0,
                      int v1_x, int v1_y, int u1, int v1, int q1,
                      int v2_x, int v2_y, int u2, int v2, int q2,
                      opacity_t opacity, precision sub_pixel_precision, precision uv_precision,
                      bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

public:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename S, typename number1=float, typename number2=float>
    void drawTriangle(const sampling::sampler<S> &sample,
                      number1 v0_x, number1 v0_y, number2 u0, number2 v0,
                      number1 v1_x, number1 v1_y, number2 u1, number2 v1,
                      number1 v2_x, number1 v2_y, number2 u2, number2 v2,
                      opacity_t opacity = 255,
                      bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename impl, typename vertex_attr, typename varying, typename number>
    void drawTriangle(shader_base<impl, vertex_attr, varying, number> &shader,
                      vertex_attr v0, vertex_attr v1, vertex_attr v2,
                      opacity_t opacity, const triangles::face_culling & culling= triangles::face_culling::none,
                       long long * depth_buffer=nullptr,
                      bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

private:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename impl, typename vertex_attr, typename varying, typename number>
    void drawTriangle_shader_homo_internal(shader_base<impl, vertex_attr, varying, number> &shader,
                                           const vec4<number> &p0, const vec4<number> &p1, const vec4<number> &p2,
                                           varying &varying_v0, varying &varying_v1, varying &varying_v2,
                                           opacity_t opacity, const triangles::face_culling & culling= triangles::face_culling::none,
                                            long long * depth_buffer=nullptr,
                                           bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

public:
    // perspective correct 2d quadrilateral defined by 2d points
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=float, typename S>
    void drawQuadrilateral(const sampling::sampler<S> &sampler,
                           number1 v0_x, number1 v0_y, number2 u0, number2 v0,
                           number1 v1_x, number1 v1_y, number2 u1, number2 v1,
                           number1 v2_x, number1 v2_y, number2 u2, number2 v2,
                           number1 v3_x, number1 v3_y, number2 u3, number2 v3,
                           opacity_t opacity = 255);
    // QUADS

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            typename number>
    void drawQuad(const color_t &color,
                  number left, number top,
                  number right, number bottom,
                  opacity_t opacity = 255);

private:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque,
            typename S>
    void drawQuad(const sampling::sampler<S> &sampler,
                  int left, int top,
                  int right, int bottom,
                  int u0, int v0,
                  int u1, int v1,
                  precision sub_pixel_precision, precision uv_precision,
                  opacity_t opacity);

public:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::SourceOverOnOpaque, typename number1=float, typename number2=number1, typename S>
    void drawQuad(const sampling::sampler<S> &sampler,
                  number1 left, number1 top,
                  number1 right, number1 bottom,
                  opacity_t opacity = 255,
                  number2 u0=number2(0), number2 v0=number2(1),
                  number2 u1=number2(1), number2 v1=number2(0));
    // Masks
    template <typename number1, typename number2=number1, typename S>
    void drawMask(const masks::chrome_mode &mode,
                  const sampling::sampler<S> &sampler,
                  number1 left, number1 top,
                  number1 right, number1 bottom,
                  number2 u0=number2(0), number2 v0=number2(1),
                  number2 u1=number2(1), number2 v1=number2(0),
                  opacity_t opacity = 255);

private:
    template <typename S>
    void drawMask(const masks::chrome_mode &mode,
                  const sampling::sampler<S> &sampler,
                  int left, int top,
                  int right, int bottom,
                  int u0, int v0,
                  int u1, int v1,
                  precision sub_pixel_precision, precision uv_precision,
                  opacity_t opacity = 255);

public:
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number1, typename number2=number1, typename S>
    void drawBezierPatch(const sampling::sampler<S> &sampler,
                         const vec3<number1> *mesh,
                         unsigned uOrder, unsigned vOrder,
                         unsigned uSamples=20, unsigned vSamples=20,
                         number2 u0=number2(0), number2 v0=number2(1),
                         number2 u1=number2(1), number2 v1=number2(0),
                         opacity_t opacity=255);

    // polygons
    template <typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::SourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, typename S>
    void drawPolygon(const sampling::sampler<S> &sampler,
                     const vec2<number1> * points,
                     index size, opacity_t opacity,
                     polygons::hints hint = polygons::hints::SIMPLE,
                     number2 u0=number2(0), number2 v0=number2(1),
                     number2 u1=number2(1), number2 v1=number2(0), bool debug=false);

    // Wu lines
    template<typename number>
    void drawLine(const color_f_t & color,
                  number x0, number y0, number x1, number y1);

private:
    void drawLine(const color_f_t & color,
                  int x0, int y0,
                  int x1, int y1,
                  uint8_t bits = 0);

public:
    template <typename number>
    void drawBezierPath(color_f_t & color, vec2<number> *points,
                         unsigned int size = 3,
                        typename tessellation::curve_divider<number>::Type type
                        = tessellation::curve_divider<number>::Type ::Quadratic,
                         typename tessellation::curve_divider<number>::CurveDivisionAlgorithm algorithm
                         = tessellation::curve_divider<number>::CurveDivisionAlgorithm::Uniform_16);
// todo: drawLinePath will be removed once the path maker is ready
    template <typename number>
    void drawLinePath(color_f_t & color,
                      vec2<number> *points,
                      unsigned int size = 4,
                      bool closed_path = false);


};

#include "../../src/Canvas.tpp"
