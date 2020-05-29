#pragma once

#include <microgl/vec2.h>
#include <microgl/matrix_3x3.h>
#include <microgl/rect.h>
#include <microgl/color.h>
#include <microgl/micro_gl_traits.h>
#include <microgl/porter_duff/FastSourceOverOnOpaque.h>
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
#include <microgl/tesselation/path.h>
#include <microgl/tesselation/ear_clipping_triangulation.h>
#include <microgl/tesselation/fan_triangulation.h>
#include <microgl/tesselation/bezier_patch_tesselator.h>
#include <microgl/cohen_sutherland_clipper.h>
#include <microgl/homo_triangle_clipper.h>
#include <microgl/functions/minmax.h>
#include <microgl/functions/clamp.h>
#include <microgl/functions/swap.h>
#include <microgl/functions/orient2d.h>

using namespace microgl::triangles;
using namespace microgl::polygons;
using namespace microgl::shading;

template<typename P, typename CODER>
class Canvas {
public:
    using rect = rect_t<int>;
    struct window_t {
        rect canvas_rect;
        rect clip_rect;
        int index_correction=0;
    };

private:

    using index = unsigned int;
    using precision = unsigned char;
    using opacity_t = unsigned char;
    using l64= long long;
    using pixel_coder= coder::PixelCoder<P, CODER>;
    using canvas= Canvas<P, CODER>;
    using bitmap= Bitmap<P, CODER>;

    bitmap * _bitmap_canvas = nullptr;
    window_t _window;
public:
    explicit Canvas(bitmap * $bmp);
    Canvas(int width, int height);

    void updateClipRect(int l, int t, int r, int b) {
        _window.clip_rect = {l, t, r, b};
    }

    void updateCanvasWindow(int left, int top, bitmap * $bmp=nullptr) {
        auto c_w = $bmp ? $bmp->width() : _window.canvas_rect.width();
        auto c_h = $bmp ? $bmp->height() : _window.canvas_rect.height();
        _window.canvas_rect = {left, top, left + c_w - 0, top + c_h - 0};
        if($bmp) _bitmap_canvas=$bmp;
        _window.index_correction= _window.canvas_rect.width()*_window.canvas_rect.top
                + _window.canvas_rect.left;
        if(_window.clip_rect.empty())
            _window.clip_rect= _window.canvas_rect;
    }

    rect calculateEffectiveDrawRect() {
        rect r = _window.canvas_rect.intersect(_window.clip_rect);
        r.bottom-=1;r.right-=1;
        return r;
    }

    const rect & clipRect() const {
        return _window.clip_rect;
    }

    inline
    int indexCorrection() const {
        return _window.index_correction;
    }

    const rect & canvasWindowRect() const {
        return _window.canvas_rect;
    }

    int width() const;
    int height() const;
    unsigned int sizeofPixel() const;
    P* pixels() const;
    P &getPixel(int x, int y) const ;
    P &getPixel(int index) const ;
    void getPixelColor(int index, color_t & output) const;
    void getPixelColor(int x, int y, color_t & output) const;

    const pixel_coder & coder() const;
    bitmap * bitmapCanvas() const;

    template <typename number>
    void clear(const intensity<number> &color);
    void clear(const color_t &color);

    // integer blenders
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::FastSourceOverOnOpaque>
    void blendColor(const color_t &val, int x, int y, opacity_t opacity);
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::FastSourceOverOnOpaque>
    inline void blendColor(const color_t &val, int index, opacity_t opacity);

    void drawPixel(const P &val, int x, int y);
    void drawPixel(const P &val, int index);

    // circles

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1, typename number2=number1, typename S1, typename S2>
    void drawCircle(const sampling::sampler<S1> & sampler_fill,
                    const sampling::sampler<S2> & sampler_stroke,
                    const number1 &centerX, const number1 &centerY,
                    const number1 &radius, const number1 &stroke_size, opacity_t opacity=255,
                    const number2 &u0=number2(0), const number2 &v0=number2(1),
                    const number2 &u1=number2(1), const number2 &v1=number2(0));

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1, typename number2=number1, typename S1, typename S2>
    void drawRoundedQuad(const sampling::sampler<S1> & sampler_fill,
                         const sampling::sampler<S2> & sampler_stroke,
                         const number1 &left, const number1 &top,
                         const number1 &right, const number1 &bottom,
                         const number1 &radius, const number1 &stroke_size,
                         opacity_t opacity=255,
                         const number2 &u0=number2(0), const number2 &v0=number2(1),
                         const number2 &u1=number2(1), const number2 &v1=number2(0));

private:
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename S1, typename S2>
    void drawRoundedQuad(const sampling::sampler<S1> & sampler_fill,
                         const sampling::sampler<S2> & sampler_stroke,
                         int left, int top,
                         int right, int bottom,
                         int radius, int stroke_size,
                         l64 u0, l64 v0, l64 u1, l64 v1,
                         precision sub_pixel_precision, precision uv_precision,
                         opacity_t opacity=255);
public:

    // Triangle batches

    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=float, typename S>
    void drawTriangles(const sampling::sampler<S> & sampler,
                       const matrix_3x3<number1> &transform,
                       const vec2<number1> *vertices= nullptr,
                       const vec2<number2> *uvs=nullptr,
                       const index *indices= nullptr,
                       const boundary_info * boundary_buffer= nullptr,
                       index size=0,
                       enum indices type=indices::TRIANGLES,
                       opacity_t opacity=255,
                       const number2 &u0=number2(0), const number2 &v0=number2(1),
                       const number2 &u1=number2(1), const number2 &v1=number2(0));

    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias, bool perspective_correct, bool depth_buffer_flag=false,
            typename impl, typename vertex_attr, typename varying, typename number>
    void drawTriangles(shader_base<impl, vertex_attr, varying, number> &shader,
                       int viewport_width, int viewport_height,
                       const vertex_attr *vertex_buffer,
                       const index *indices,
                       const boundary_info * boundary_buffer,
                       index size,
                       enum indices type,
                       const triangles::face_culling & culling= triangles::face_culling::none,
                       long long * depth_buffer=nullptr,
                       opacity_t opacity=255);

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number=float>
    void drawTrianglesWireframe(const color_t & color,
                                const matrix_3x3<number> &transform,
                                const vec2<number> *vertices,
                                const index *indices,
                                index size,
                                enum indices type,
                                opacity_t opacity=255);

    // single triangles, includes shader based and one very fast fixed pipeline for color and textures
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number=float>
    void drawTriangleWireframe(const color_t &color,
                               const vec2<number> &p0,
                               const vec2<number> &p1,
                               const vec2<number> &p2,
                               opacity_t opacity=255);

private:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
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
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, typename S>
    void drawTriangle(const sampling::sampler<S> &sample,
                      const number1 &v0_x, const number1 &v0_y, const number2 &u0, const number2 &v0,
                      const number1 &v1_x, const number1 &v1_y, const number2 &u1, const number2 &v1,
                      const number1 &v2_x, const number1 &v2_y, const number2 &u2, const number2 &v2,
                      opacity_t opacity = 255, bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None<>,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename impl, typename vertex_attr, typename varying, typename number>
    void drawTriangle(shader_base<impl, vertex_attr, varying, number> &shader,
                      int viewport_width, int viewport_height,
                      vertex_attr v0, vertex_attr v1, vertex_attr v2,
                      opacity_t opacity, const triangles::face_culling & culling= triangles::face_culling::none,
                      long long * depth_buffer=nullptr,
                      bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

private:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None<>,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename impl, typename vertex_attr, typename varying, typename number>
    void drawTriangle_shader_homo_internal(shader_base<impl, vertex_attr, varying, number> &shader,
                                           int viewport_width, int viewport_height,
                                           const vec4<number> &p0, const vec4<number> &p1, const vec4<number> &p2,
                                           varying &varying_v0, varying &varying_v1, varying &varying_v2,
                                           opacity_t opacity, const triangles::face_culling & culling= triangles::face_culling::none,
                                            long long * depth_buffer=nullptr,
                                           bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

public:
    // perspective correct 2d quadrilateral defined by 2d points
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, typename S>
    void drawQuadrilateral(const sampling::sampler<S> &sampler,
                           const number1 & v0_x, const number1 & v0_y, const number2 & u0, const number2 & v0,
                           const number1 & v1_x, const number1 & v1_y, const number2 & u1, const number2 & v1,
                           const number1 & v2_x, const number1 & v2_y, const number2 & u2, const number2 & v2,
                           const number1 & v3_x, const number1 & v3_y, const number2 & u3, const number2 & v3,
                           opacity_t opacity = 255);
    // QUADS

private:
    template <typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename S>
    void drawQuad(const sampling::sampler<S> &sampler,
                  int left, int top,
                  int right, int bottom,
                  int u0, int v0,
                  int u1, int v1,
                  precision sub_pixel_precision, precision uv_precision,
                  opacity_t opacity);

public:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1=float, typename number2=number1, typename S>
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
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1, typename number2=number1, typename S>
    void drawBezierPatch(const sampling::sampler<S> &sampler,
                         const matrix_3x3<number1> &transform,
                         const vec3<number1> *mesh,
                         unsigned uOrder, unsigned vOrder,
                         unsigned uSamples=20, unsigned vSamples=20,
                         number2 u0=number2(0), number2 v0=number2(1),
                         number2 u1=number2(1), number2 v1=number2(0),
                         opacity_t opacity=255);

    // polygons
    template <typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, typename S>
    void drawPolygon(const sampling::sampler<S> &sampler,
                     const matrix_3x3<number1> &transform,
                     const vec2<number1> * points,
                     index size, opacity_t opacity=255,
                     polygons::hints hint = polygons::hints::SIMPLE,
                     number2 u0=number2(0), number2 v0=number2(1),
                     number2 u1=number2(1), number2 v1=number2(0), bool debug=false);
private:
    void drawWuLine(const color_t & color,
                    int x0, int y0, int x1, int y1,
                    precision bits = 0, opacity_t opacity=255);

public:
    // Wu lines
    template<typename number>
    void drawWuLine(const color_t & color,
                    const number &x0, const number &y0, const number &x1, const number &y1,
                    opacity_t opacity=255);

    template <typename number>
    void drawWuLinePath(const color_t & color,
                        const vec2<number> *points,
                        unsigned int size = 4,
                        bool closed_path = false);

    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=float, typename S>
    void drawPathStroke(const sampling::sampler<S> &sampler,
                        const matrix_3x3<number1> &transform,
                        tessellation::path<number1> &path,
                        const number1 &stroke_width=number1(1),
                        const tessellation::stroke_cap &cap=tessellation::stroke_cap::butt,
                        const tessellation::stroke_line_join &line_join=tessellation::stroke_line_join::bevel,
                        int miter_limit=4,
                        const std::initializer_list<int> & stroke_dash_array={},
                        int stroke_dash_offset=0, opacity_t opacity=255,
                        number2 u0=number2(0), number2 v0=number2(1),
                        number2 u1=number2(1), number2 v1=number2(0), bool debug=false);

    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=float, typename S>
    void drawPathFill(const sampling::sampler<S> &sampler,
                      const matrix_3x3<number1> &transform,
                      tessellation::path<number1> &path,
                      const tessellation::fill_rule &rule=tessellation::fill_rule::non_zero,
                      const tessellation::tess_quality &quality=tessellation::tess_quality::better,
                      opacity_t opacity=255,
                      number2 u0=number2(0), number2 v0=number2(1),
                      number2 u1=number2(1), number2 v1=number2(0), bool debug=false);
};

#include "../../src/Canvas.tpp"
