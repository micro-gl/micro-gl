#pragma once

#include <microgl/text/bitmap_font.h>
#include <microgl/vec2.h>
#include <microgl/matrix_3x3.h>
#include <microgl/rect.h>
#include <microgl/color.h>
#include <microgl/pixel_coder.h>
#include <microgl/micro_gl_traits.h>
#include <microgl/porter_duff/FastSourceOverOnOpaque.h>
#include <microgl/porter_duff/DestinationIn.h>
#include <microgl/porter_duff/None.h>
#include <microgl/blend_modes/Normal.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/shader.h>
#include <microgl/sampler.h>
#include <microgl/triangles.h>
#include <microgl/polygons.h>
#include <microgl/masks.h>
#include <microgl/rgba_t.h>
#include <microgl/math.h>
#include <microgl/tesselation/path.h>
#include <microgl/tesselation/monotone_polygon_triangulation.h>
#include <microgl/tesselation/ear_clipping_triangulation.h>
#include <microgl/tesselation/fan_triangulation.h>
#include <microgl/tesselation/bezier_patch_tesselator.h>
#include <microgl/cohen_sutherland_clipper.h>
#include <microgl/homo_triangle_clipper.h>
#include <microgl/functions/minmax.h>
#include <microgl/functions/clamp.h>
#include <microgl/functions/swap.h>
#include <microgl/functions/orient2d.h>
#include <microgl/functions/bits.h>

using namespace microgl::triangles;
using namespace microgl::polygons;
using namespace microgl::shading;

/**
 * use big integers for 2d rasterizer, this implies a 64 bits place holders
 * for all or most calculations inside the rasterizer. bigger integers imply
 * overflow is harder to come by
 */
#define CANVAS_OPT_2d_raster_USE_BIG_INT uint8_t(0b00000001)
/**
 * inside the 2d rasterizer, use division for uv-mapping, this reduces
 * the number of bits used BUT is slower. Generally this HAS to be used
 * on a forced 32 bit rasterizer, in case you want a pure 32 bit integers
 * only during rasterization. Do not use it when in BIG INT mode.
 */
#define CANVAS_OPT_2d_raster_USE_DIVISION uint8_t(0b00000010)
/**
 * inside the rasterizers, allow some bit compression in-order to minimize
 * overflow in 32 bit mode. disable it if 3d rendering for example starts
 * to jitter.
 */
#define CANVAS_OPT_raster_COMPRESS_BITS uint8_t(0b00001000)
/**
 * the 2d and 3d rasterizer can detect overflow of uv mapping, the detection
 * feature is great for debugging the rasterizer. this flag enables detection
 * and if so, exits the rendering. This is helpful for when using a 32 bit mode,
 * where overflows are likely to happen
 */
#define CANVAS_OPT_2d_raster_AVOID_RENDER_WITH_OVERFLOWS uint8_t(0b00000100)
/**
 * use a true 32 bit mode in the 2d and 3d rasterizer, this means regular 32 bit integers
 * and also the usage of division in order to reduce overflow and also detecting
 * and exiting on overflows as they are likely to happen in 32 bit mode, if so,
 * please adjust some of the render options bits in the canvas and make sure you
 * render small geometries at a time
 */
#define CANVAS_OPT_2d_raster_FORCE_32_BIT (CANVAS_OPT_2d_raster_USE_DIVISION | \
                        CANVAS_OPT_2d_raster_AVOID_RENDER_WITH_OVERFLOWS | CANVAS_OPT_raster_COMPRESS_BITS)
/**
 * default preset, includes usage of big integers
 */
#define CANVAS_OPT_2d_raster_FORCE_64_BIT (CANVAS_OPT_2d_raster_USE_BIG_INT | \
                            CANVAS_OPT_2d_raster_AVOID_RENDER_WITH_OVERFLOWS | \
                            CANVAS_OPT_raster_COMPRESS_BITS )
/**
 * default preset, includes usage of big integers
 */
#define CANVAS_OPT_default CANVAS_OPT_2d_raster_FORCE_64_BIT

/**
 * the main canvas object:
 * - holds minimal state
 * - holds a bitmap reference
 *
 * @tparam bitmap_type the bitmap type
 * @tparam options the options bitset
 */
template<typename bitmap_type, uint8_t options=CANVAS_OPT_default>
class canvas {
public:
    using rect = rect_t<int>;
    using canvas_t = canvas<bitmap_type, options>;
    using bitmap_t = bitmap_type;
    using pixel = typename bitmap_type::pixel;
    using pixel_coder = typename bitmap_type::pixel_coder;

    struct window_t {
        rect canvas_rect;
        rect clip_rect;
        int index_correction=0;
    };

    static constexpr bool options_compress_bits() { return options&CANVAS_OPT_raster_COMPRESS_BITS; }
    static constexpr bool options_big_integers() { return options&CANVAS_OPT_2d_raster_USE_BIG_INT; }
    static constexpr bool options_avoid_overflow() { return options&CANVAS_OPT_2d_raster_AVOID_RENDER_WITH_OVERFLOWS; }
    static constexpr bool options_use_division() { return options&CANVAS_OPT_2d_raster_USE_DIVISION; }
    static constexpr bool hasNativeAlphaChannel() { return pixel_coder::rgba::a != 0;}

    /**
     * rendering options of rasterizer
     */
    struct render_options_t {
        uint8_t _2d_raster_bits_sub_pixel= options_big_integers() ? 8 : 4;
        uint8_t _2d_raster_bits_uv= options_big_integers() ? 15 : 10;
        uint8_t _3d_raster_bits_sub_pixel= options_big_integers() ? 8 : 4;
        uint8_t _3d_raster_bits_w= options_big_integers() ? 15 : 12;
    };

private:
    using index = unsigned int;
    using precision = unsigned char;
    using opacity_t = unsigned char;
    using l64= long long;
    // rasterizer integers
    using rint_big = int64_t;
    using rint =typename microgl::traits::conditional<
            options_big_integers(), rint_big, int32_t >::type;

    bitmap_type * _bitmap_canvas = nullptr;
    window_t _window;
    render_options_t _options;
public:

    /**
     * ctor of canvas that receives a bitmap reference
     *
     * @param $bmp a bitmap reference
     */
    explicit canvas(bitmap_type * $bmp);
    /**
     * ctor of canvas that allocate a bitmap internally
     * @param width     width of canvas
     * @param height    height of canvas
     */
    canvas(int width, int height);

    /**
     * update the clipping rectangle of the canvas
     *
     * @param l left distance to x=0
     * @param t top distance to y=0
     * @param r right distance to x=0
     * @param b bottom distance to y=0
     */
    void updateClipRect(int l, int t, int r, int b) {
        _window.clip_rect = {l, t, r, b};
    }

    /**
     * where to position the bitmap relative to the canvas, this feature
     * can help with block rendering, where the bitmap is smaller than the canvas
     * diensions.
     *
     * @param left relative to x=0
     * @param top relative to y=0
     * @param $bmp (Optional) the bitmap reference
     */
    void updateCanvasWindow(int left, int top, bitmap_type * $bmp=nullptr) {
        auto c_w = $bmp ? $bmp->width() : _window.canvas_rect.width();
        auto c_h = $bmp ? $bmp->height() : _window.canvas_rect.height();
        _window.canvas_rect = {left, top, left + c_w - 0, top + c_h - 0};
        if($bmp) _bitmap_canvas=$bmp;
        _window.index_correction= _window.canvas_rect.width()*_window.canvas_rect.top
                + _window.canvas_rect.left;
        if(_window.clip_rect.empty())
            _window.clip_rect= _window.canvas_rect;
    }

    /**
     * given that we know the canvas size and the clip rect, calculate
     * the sub rectangle (intersection), where drawing is visible
     *
     * @return a rectangle
     */
    rect calculateEffectiveDrawRect() {
        rect r = _window.canvas_rect.intersect(_window.clip_rect);
        r.bottom-=1;r.right-=1;
        return r;
    }

    /**
     * get the clipping rectangle
     * @return rect reference
     */
    const rect & clipRect() const {
        return _window.clip_rect;
    }

    /**
     * when using the canvas window feature, index should be offset.
     * @return the offset
     */
    inline
    int indexCorrection() const {
        return _window.index_correction;
    }

    /**
     * get the canvas rectangle, should be (0, 0, width, height), unless
     * the sub windowing feature was used.
     * @return a rectangle
     */
    const rect & canvasWindowRect() const {
        return _window.canvas_rect;
    }

    /**
     * get the rendering options
     * @return options object
     */
    render_options_t & renderingOptions() {
        return _options;
    }

    // get canvas width
    int width() const;
    // get canvas height
    int height() const;
    // get size of pixel
    unsigned int sizeofPixel() const;
    // get the pixels array from the underlying bitmap
    pixel * pixels() const;
    // get a pixel by position
    pixel &getPixel(int x, int y) const ;
    pixel &getPixel(int index) const ;
    // decode pixel color by position
    void getPixelColor(int index, color_t & output) const;
    void getPixelColor(int x, int y, color_t & output) const;

    /**
     * get the pixel coder reference of the underlying bitmap
     * @return pixel_coder
     */
    const pixel_coder & coder() const;
    /**
     * get the underlying bitmap pointer
     */
    bitmap_type * bitmapCanvas() const;

    /**
     * clear the canvas with a color intensity
     * @tparam number the number type of the intensity
     * @param color the color intensity
     */
    template <typename number>
    void clear(const intensity<number> &color);
    /**
     * clear the canvas with a color of the same type of the canvas
     */
    void clear(const color_t &color);

    // integer blenders
    /**
     * blend and composite a given color at position to the backdrop of the canvas
     *
     * @tparam BlendMode the blend mode type
     * @tparam PorterDuff the alpha compositing type
     * @tparam a_src the bits of the alpha channel of the color
     * @param val the color to blend
     * @param index the position of where to compose in the canvas
     * @param opacity 8 bit opacity [0..255]
     */
    template<typename BlendMode=blendmode::Normal,
             typename PorterDuff=porterduff::FastSourceOverOnOpaque,
             uint8_t a_src>
    inline void blendColor(const color_t &val, int index, opacity_t opacity);
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            uint8_t a_src>
    void blendColor(const color_t &val, int x, int y, opacity_t opacity);

    /**
     * draw an already encoded pixel at position
     */
    void drawPixel(const pixel &val, int index);
    void drawPixel(const pixel &val, int x, int y);

    // circles

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1, typename number2=number1, typename Sampler1, typename Sampler2>
    void drawCircle(const Sampler1 & sampler_fill,
                    const Sampler2 & sampler_stroke,
                    const number1 &centerX, const number1 &centerY,
                    const number1 &radius, const number1 &stroke_size, opacity_t opacity=255,
                    const number2 &u0=number2(0), const number2 &v0=number2(1),
                    const number2 &u1=number2(1), const number2 &v1=number2(0));

    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false, typename number1,
            typename number2=number1, typename Sampler1, typename Sampler2>
    void drawRoundedRect(const Sampler1 & sampler_fill,
                         const Sampler2 & sampler_stroke,
                         const number1 &left, const number1 &top,
                         const number1 &right, const number1 &bottom,
                         const number1 &radius, const number1 &stroke_size,
                         opacity_t opacity= 255,
                         const number2 &u0= number2(0), const number2 &v0= number2(1),
                         const number2 &u1= number2(1), const number2 &v1= number2(0));

private:
    template<typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename Sampler1, typename Sampler2>
    void drawRoundedRect(const Sampler1 & sampler_fill,
                         const Sampler2 & sampler_stroke,
                         int left, int top,
                         int right, int bottom,
                         int radius, int stroke_size,
                         int u0, int v0, int u1, int v1,
                         precision sub_pixel_precision, precision uv_precision,
                         opacity_t opacity= 255);

private:
    template <typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename Sampler>
    void drawRect(const Sampler &sampler,
                  int left, int top,
                  int right, int bottom,
                  int u0, int v0,
                  int u1, int v1,
                  precision sub_pixel_precision, precision uv_precision,
                  opacity_t opacity);

public:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1=float, typename number2=number1, typename Sampler>
    void drawRect(const Sampler &sampler,
                  number1 left, number1 top,
                  number1 right, number1 bottom,
                  opacity_t opacity = 255,
                  number2 u0= number2(0), number2 v0= number2(1),
                  number2 u1= number2(1), number2 v1= number2(0));

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false,
            typename number1=float, typename number2=number1, typename Sampler>
    void drawRect(const Sampler &sampler,
                  const matrix_3x3<number1> &transform,
                  number1 left, number1 top,
                  number1 right, number1 bottom,
                  opacity_t opacity = 255,
                  number2 u0= number2(0), number2 v0= number2(1),
                  number2 u1= number2(1), number2 v1= number2(0));

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, class Sampler>
    void drawQuadrilateral(const Sampler &sampler,
                           const number1 & v0_x, const number1 & v0_y, const number2 & u0, const number2 & v0,
                           const number1 & v1_x, const number1 & v1_y, const number2 & u1, const number2 & v1,
                           const number1 & v2_x, const number1 & v2_y, const number2 & u2, const number2 & v2,
                           const number1 & v3_x, const number1 & v3_y, const number2 & u3, const number2 & v3,
                           opacity_t opacity = 255);

public:

    // Triangle batches

    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=float, typename Sampler>
    void drawTriangles(const Sampler & sampler,
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
            typename Shader, typename depth_buffer_type >
    void drawTriangles(Shader &shader,
                       int viewport_width, int viewport_height,
                       const vertex_attributes<Shader> *vertex_buffer,
                       const index *indices,
                       index size,
                       enum indices type,
                       const triangles::face_culling & culling= triangles::face_culling::none,
                       depth_buffer_type *depth_buffer=(nullptr),
                       opacity_t opacity=255,
                       const shader_number<Shader>& depth_range_near=shader_number<Shader>(0),
                       const shader_number<Shader>& depth_range_far=shader_number<Shader>(1));

    void fxaa(int left, int top, int right, int bottom);

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
    template<typename number=float>
    void drawTriangleWireframe(const color_t &color,
                               const vec2<number> &p0,
                               const vec2<number> &p1,
                               const vec2<number> &p2,
                               opacity_t opacity=255);

private:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool perspective_correct=false,
            typename Sampler>
    void drawTriangle(const Sampler &sample,
                      int v0_x, int v0_y, int u0, int v0, int q0,
                      int v1_x, int v1_y, int u1, int v1, int q1,
                      int v2_x, int v2_y, int u2, int v2, int q2,
                      opacity_t opacity, precision sub_pixel_precision, precision uv_precision,
                      bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

public:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, typename number1=float, typename number2=number1, typename Sampler>
    void drawTriangle(const Sampler &sample,
                      const number1 &v0_x, const number1 &v0_y, const number2 &u0, const number2 &v0,
                      const number1 &v1_x, const number1 &v1_y, const number2 &u1, const number2 &v1,
                      const number1 &v2_x, const number1 &v2_y, const number2 &u2, const number2 &v2,
                      opacity_t opacity = 255, bool aa_first_edge = true, bool aa_second_edge = true, bool aa_third_edge = true);

    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None<>,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename Shader, typename depth_buffer_type >
    void drawTriangle(Shader &shader,
                      int viewport_width, int viewport_height,
                      vertex_attributes<Shader> v0,
                      vertex_attributes<Shader> v1,
                      vertex_attributes<Shader> v2,
                      opacity_t opacity, const triangles::face_culling & culling= triangles::face_culling::none,
                      depth_buffer_type * depth_buffer=nullptr,
                      const shader_number<Shader>& depth_range_near=shader_number<Shader>(0),
                      const shader_number<Shader>& depth_range_far=shader_number<Shader>(1));

private:
    template <typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::None<>,
            bool antialias=true, bool perspective_correct=false, bool depth_buffer_flag=false,
            typename Shader, typename number, typename depth_buffer_type >
    void drawTriangle_shader_homo_internal(Shader &$shader,
                                           int viewport_width, int viewport_height,
                                           const vec4<number> &p0, const vec4<number> &p1, const vec4<number> &p2,
                                           varying<Shader> varying_v0,
                                           varying<Shader> varying_v1,
                                           varying<Shader> varying_v2,
                                           opacity_t opacity, const triangles::face_culling & culling= triangles::face_culling::none,
                                           depth_buffer_type * depth_buffer=nullptr,
                                           number depth_range_near=number(0), number depth_range_far=number(1));

public:
    // Masks
    template <typename number1, typename number2=number1, typename S>
    void drawMask(const masks::chrome_mode &mode,
                  const S &sampler,
                  number1 left, number1 top,
                  number1 right, number1 bottom,
                  number2 u0=number2(0), number2 v0=number2(1),
                  number2 u1=number2(1), number2 v1=number2(0),
                  opacity_t opacity = 255);

private:
    template <typename S>
    void drawMask(const masks::chrome_mode &mode,
                  const S &sampler,
                  int left, int top,
                  int right, int bottom,
                  int u0, int v0,
                  int u1, int v1,
                  precision sub_pixel_precision, precision uv_precision,
                  opacity_t opacity = 255);

public:
    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool debug=false, typename number1, typename number2=number1, typename Sampler>
    void drawBezierPatch(const Sampler &sampler,
                         const matrix_3x3<number1> &transform,
                         const vec3<number1> *mesh,
                         unsigned uOrder, unsigned vOrder,
                         unsigned uSamples=20, unsigned vSamples=20,
                         number2 u0=number2(0), number2 v0=number2(1),
                         number2 u1=number2(1), number2 v1=number2(0),
                         opacity_t opacity=255);

    // polygons
    template <microgl::polygons::hints hint=polygons::hints::SIMPLE, typename BlendMode=blendmode::Normal,
            typename PorterDuff=porterduff::FastSourceOverOnOpaque, bool antialias=false, bool debug=false,
            typename number1=float, typename number2=number1, typename Sampler>
    void drawPolygon(const Sampler &sampler,
                     const matrix_3x3<number1> &transform,
                     const vec2<number1> * points,
                     index size, opacity_t opacity=255,
                     number2 u0=number2(0), number2 v0=number2(1),
                     number2 u1=number2(1), number2 v1=number2(0));
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
            bool antialias=false, bool debug=false, typename number1=float, typename number2=float, typename Sampler>
    void drawPathStroke(const Sampler &sampler,
                        const matrix_3x3<number1> &transform,
                        tessellation::path<number1> &path,
                        const number1 &stroke_width=number1(1),
                        const tessellation::stroke_cap &cap=tessellation::stroke_cap::butt,
                        const tessellation::stroke_line_join &line_join=tessellation::stroke_line_join::bevel,
                        int miter_limit=4,
                        const std::initializer_list<int> & stroke_dash_array={},
                        int stroke_dash_offset=0, opacity_t opacity=255,
                        number2 u0=number2(0), number2 v0=number2(1),
                        number2 u1=number2(1), number2 v1=number2(0));

    template<typename BlendMode=blendmode::Normal, typename PorterDuff=porterduff::FastSourceOverOnOpaque,
            bool antialias=false, bool debug=false, typename number1=float, typename number2=float, typename Sampler>
    void drawPathFill(const Sampler &sampler,
                      const matrix_3x3<number1> &transform,
                      tessellation::path<number1> &path,
                      const tessellation::fill_rule &rule=tessellation::fill_rule::non_zero,
                      const tessellation::tess_quality &quality=tessellation::tess_quality::better,
                      opacity_t opacity=255,
                      number2 u0=number2(0), number2 v0=number2(1),
                      number2 u1=number2(1), number2 v1=number2(0));

    template<bool tint=true, typename BITMAP_FONT_TYPE>
    void drawText(const char *text, microgl::text::bitmap_font<BITMAP_FONT_TYPE> &font, const color_t & color,
            microgl::text::text_format & format,
            int left, int top, int right, int bottom, bool frame, opacity_t opacity=255);
};

#include "canvas.tpp"
