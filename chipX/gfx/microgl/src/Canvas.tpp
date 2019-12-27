
//#include "../include/public/microgl/Canvas.h"

//#include <microgl/Canvas.h>


#include <microgl/tesselation/ear_clipping_triangulation.h>
#include <microgl/Canvas.h>


template<typename P, typename CODER>
Canvas<P, CODER>::Canvas(Bitmap<P, CODER> *$bmp)
                        : _width{$bmp->width()}, _height{$bmp->height()}, _bitmap_canvas($bmp) {


    uint8_t alpha_bits = coder()->bits_per_alpha();

    _flag_hasNativeAlphaChannel = alpha_bits!=0;

    // fix alpha bits depth in case we don't natively
    // support alpha, this is correct because we want to
    // support compositing even if the surface is opaque.

    _alpha_bits_for_compositing = _flag_hasNativeAlphaChannel ? alpha_bits : 8;
    _max_alpha_value = (1<<_alpha_bits_for_compositing) - 1;
}

template<typename P, typename CODER>
Canvas<P, CODER>::Canvas(int width, int height, PixelCoder<P, CODER> * $coder) :
            Canvas<P, CODER>(new Bitmap<P, CODER>(width, height, $coder)) {

}

template<typename P, typename CODER>
inline PixelCoder<P, CODER> *Canvas<P, CODER>::coder() {
    return _bitmap_canvas->coder();
}

template<typename P, typename CODER>
inline Bitmap<P, CODER> *Canvas<P, CODER>::bitmapCanvas() {
    return _bitmap_canvas;
}

template<typename P, typename CODER>
PixelFormat Canvas<P, CODER>::pixelFormat() {
    return coder()->format();
}

template<typename P, typename CODER>
unsigned int Canvas<P, CODER>::sizeofPixel() {
    return sizeof(P{});
}

template<typename P, typename CODER>
P &Canvas<P, CODER>::getPixel(int x, int y) {
    // this is not good for high performance loop, cannot be inlined
    return _bitmap_canvas->readAt(y*_width + x);
}

template<typename P, typename CODER>
P &Canvas<P, CODER>::getPixel(int index) {
    // this is not good for high performance loop, cannot be inlined
    return _bitmap_canvas->readAt(index);
}

template<typename P, typename CODER>
bool Canvas<P, CODER>::hasNativeAlphaChannel() {
    return _flag_hasNativeAlphaChannel;
}

template<typename P, typename CODER>
bool Canvas<P, CODER>::hasAntialiasing() {
    return _flag_antiAlias;
}

template<typename P, typename CODER>
void Canvas<P, CODER>::setAntialiasing(bool value) {
    _flag_antiAlias = value;
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int x, int y, color_t & output) {
    this->_bitmap_canvas->decode(x, y, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int index, color_t & output) {
    this->_bitmap_canvas->decode(index, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int x, int y, color_f_t & output) {
    this->_bitmap_canvas->decode(x, y, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int index, color_f_t & output) {
    this->_bitmap_canvas->decode(index, output);
}

template<typename P, typename CODER>
int Canvas<P, CODER>::width() {
    return _width;
}

template<typename P, typename CODER>
int Canvas<P, CODER>::height() {
    return _height;
}

template<typename P, typename CODER>
P *Canvas<P, CODER>::pixels() {
    return _bitmap_canvas->data();
}

template<typename P, typename CODER>
void Canvas<P, CODER>::clear(const color_f_t &color) {
    P output;
    _bitmap_canvas->coder()->encode(color, output);
    _bitmap_canvas->fill(output);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_f_t &val, int x, int y, float opacity) {
    blendColor<BlendMode, PorterDuff>(val, y*_width + x, opacity);
}


template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_f_t &val, int index, float opacity) {
    color_f_t result;//=val;
    P output;

    if(true){
        color_f_t backdrop, blended;
        const color_f_t & src = val;

        // get backdrop color
        getPixelColor(index, backdrop);

        uint8_t alpha_bits = coder()->bits_per_alpha();

        // fix alpha bits depth in case we don't natively
        // support alpha, this is correct because we want to
        // support compositing even if the surface is opaque.
        if(alpha_bits==0) {
            backdrop.a = 1.0f;
        }

        // if blend-mode is normal or the backdrop is completely transparent
        // then we don't need to blend
        bool skip_blending = BlendMode::type()==blendmode::type::Normal || backdrop.a==0;

        // if we are normal then do nothing
        if(!skip_blending) { //  or backdrop alpha is zero is also valid

            BlendMode::blend(backdrop, src, blended);

            // if backdrop alpha!= max_alpha let's first composite the blended color, this is
            // an intermidiate step before Porter-Duff
            if(backdrop.a < 1.0f) {
                float comp = 1.0f - backdrop.a;
                blended.r = (comp * src.r + backdrop.a * blended.r);
                blended.g = (comp * src.g + backdrop.a * blended.g);
                blended.b = (comp * src.b + backdrop.a * blended.b);
            }
            else {
                // do nothing if background is opaque (backdrop alpha==max_alpha) then it will equal blended
            }

        }
        else {
            // skipped blending therefore use src color
            blended.r = src.r;
            blended.g = src.g;
            blended.b = src.b;
        }

        // preserve src alpha before Porter-Duff
        blended.a = src.a * opacity;

        // finally alpha composite with Porter-Duff equations
        PorterDuff::composite(backdrop, blended, result);

    } else
        result = val;

    coder()->encode(result, output);

    drawPixel(output, index);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_t &val, int x, int y, uint8_t opacity) {
    blendColor<BlendMode, PorterDuff>(val, y*_width + x, opacity);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_t &val, int index, uint8_t opacity) {
    color_t result;//=val;
    P output;

    if(true){
        color_t backdrop, blended;
        const color_t & src = val;

        // get backdrop color
        getPixelColor(index, backdrop);

        uint8_t alpha_bits = coder()->bits_per_alpha();

        // fix alpha bits depth in case we don't natively
        // support alpha, this is correct because we want to
        // support compositing even if the surface is opaque.
        if(!hasNativeAlphaChannel()) {
            backdrop.a = _max_alpha_value; alpha_bits = _alpha_bits_for_compositing;
        }

        // if blend-mode is normal or the backdrop is completely transparent
        // then we don't need to blend.
        // the first conditional shouldbe resolved at compile-time therefore it is zero cost !!!
        // this will help with avoiding the inner conditional of the backdrop alpha, the normal
        // blending itself is zero-cost itself, but after it there is a branching which
        // is unpredictable, therefore avoiding at compile-time is great.
        bool skip_blending = BlendMode::type()==blendmode::type::Normal || backdrop.a==0;

        // if we are normal then do nothing
        if(!skip_blending) { //  or backdrop alpha is zero is also valid

            BlendMode::blend(backdrop, src, blended,
                             coder()->bits_per_red(),
                             coder()->bits_per_green(),
                             coder()->bits_per_blue());

            // if backdrop alpha!= max_alpha let's first composite the blended color, this is
            // an intermidiate step before Porter-Duff
            if(backdrop.a < _max_alpha_value) {
                // if((backdrop.a ^ _max_alpha_value)) {
                int max_alpha = _max_alpha_value;
                unsigned int comp = max_alpha - backdrop.a;
                blended.r = (comp * src.r + backdrop.a * blended.r) >> alpha_bits;
                blended.g = (comp * src.g + backdrop.a * blended.g) >> alpha_bits;
                blended.b = (comp * src.b + backdrop.a * blended.b) >> alpha_bits;
            }
            else {
                // do nothing if background is opaque (backdrop alpha==max_alpha) then it will equal blended
            }

        }
        else {
            // skipped blending therefore use src color
            blended.r = src.r;
            blended.g = src.g;
            blended.b = src.b;
        }

        // preserve src alpha before Porter-Duff
        blended.a = src.a;

        // apply opacity
        // I fixed opacity is always 8 bits no matter what the alpha depth of the
        // native canvas
        if(opacity < 255)
            blended.a =  (blended.a * opacity) >> 8;

        // finally alpha composite with Porter-Duff equations,
        // this should be zero-cost for None option with compiler optimizations
        PorterDuff::composite(backdrop, blended, result, alpha_bits);
    } else
        result = val;

    coder()->encode(result, output);

    drawPixel(output, index);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawPixel(const P & val, int x, int y) {
    int index = (y * width() + x);
    _bitmap_canvas->writeAt(val, index);
}

template<typename P, typename CODER>
inline void Canvas<P, CODER>::drawPixel(const P & val, int index) {
    _bitmap_canvas->writeAt(val, index);
}

// Circles

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawCircle(const color_f_t & color,
                                  const int centerX,
                                  const int centerY,
                                  const int radius,
                                  const precision sub_pixel_precision,
                                  const opacity opacity) {
    color_t color_int;
    uint8_t p = sub_pixel_precision;
    coder()->convert(color, color_int);

    int bits_for_antialias_distance, max_blend_distance=0;
    int a, b, c=0;

    if(antialias) {
        bits_for_antialias_distance = 1;
        max_blend_distance = (1u << bits_for_antialias_distance)<<(p);
        a = fixed_mul_fixed_2(radius, radius, p);
        b = fixed_mul_fixed_2(radius+max_blend_distance, radius+max_blend_distance, p);
        c = b - a;
    }

    bool apply_opacity = opacity!=255;
    int delta;

    // bounding box
    int x_min = centerX - radius - max_blend_distance, y_min = centerY - radius - max_blend_distance;
    int x_max = centerX + radius + max_blend_distance, y_max = centerY + radius + max_blend_distance;

    // clipping
    x_min = functions::max(0, x_min);
    y_min = functions::max(0, y_min);
    x_max = functions::min((int)int_to_fixed_2(width()-0, p), x_max);
    y_max = functions::min((int)int_to_fixed_2(height()-0, p), y_max);
    int step = functions::max((1<<p)-0, 1);

    // Round start position up to next integer multiple
    // (we sample at integer pixel positions, so if our
    // min is not an integer coordinate, that pixel won't
    // be hit)
    fixed_signed sub_mask = (step-1);
    x_min = (x_min + sub_mask) & (~sub_mask);
    y_min = (y_min + sub_mask) & (~sub_mask);
    x_max = (x_max + sub_mask) & (~sub_mask);
    y_max = (y_max + sub_mask) & (~sub_mask);

    for (int y = y_min; y < y_max; y+=step) {

        for (int x = x_min; x < x_max; x+=step) {

            // 16 bit precision fixed point
            int distance = functions::signed_distance_circle_raised_quad(x, y, centerX, centerY, radius, p);

            if(distance<=0)
                blendColor<BlendMode, PorterDuff>(color_int, x>>p, y>>p, opacity);
            else if(antialias && (delta=c-distance)>=0){

                // scale inner to 8 bit and then convert to integer
                uint8_t blend = ((delta)<<(8))/c;

                if(apply_opacity)
                    blend = (blend*opacity)>>8;

                blendColor<BlendMode, PorterDuff>(color_int, (x>>p), y>>p, blend);
            }

        }

    }

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawCircle(const color_f_t & color,
                                  const number centerX,
                                  const number centerY,
                                  const number radius,
                                  opacity opacity) {

    precision p = 10;

    drawCircle<BlendMode, PorterDuff, antialias>(color,
                microgl::math::to_fixed(centerX, p),
                microgl::math::to_fixed(centerY, p),
                microgl::math::to_fixed(radius, p),
                p, opacity
                );

}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawGradient(const color_f_t & startColor,
                             const color_f_t & endColor,
                             int left, int top, int w, int h) {
    float t;
    color_f_t res{};

    for (int x = left; x < left + w; ++x) {

        t = float(x - left) / (w);

        float r = functions::lerp(startColor.r, endColor.r, t);
        float g = functions::lerp(startColor.g, endColor.g, t);
        float b = functions::lerp(startColor.b, endColor.b, t);
        float a = functions::lerp(startColor.a, endColor.a, t);

        res = {r, g, b, a};

        drawQuad(res, x, top, x+ 1, top+h);
    }

}

// Triangles

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTriangles(const color_f_t &color,
                                     const vec2<number> *vertices,
                                     const index *indices,
                                     const boundary_info * boundary_buffer,
                                     const index size,
                                     const TrianglesIndices type,
                                     const opacity opacity) {

#define IND(a) indices[(a)]
#define to_fixed microgl::math::to_fixed
    const precision p = 4;

    switch (type) {
        case TrianglesIndices::TRIANGLES:

            for (index ix = 0; ix < size; ix+=3) {

                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                        to_fixed(vertices[IND(ix + 0)].x, p), to_fixed(vertices[IND(ix + 0)].y, p),
                        to_fixed(vertices[IND(ix + 1)].x, p), to_fixed(vertices[IND(ix + 1)].y, p),
                        to_fixed(vertices[IND(ix + 2)].x, p), to_fixed(vertices[IND(ix + 2)].y, p),
                        opacity, p
                );
            }

            break;
        case TrianglesIndices::TRIANGLES_WITH_BOUNDARY:
        {
            index idx_boundary=0;
            for (index ix = 0; ix < size; ix+=3) {
                boundary_info aa_info = boundary_buffer[idx_boundary++];

                bool aa_first_edge = triangles::classify_boundary_info(aa_info, 0);
                bool aa_second_edge = triangles::classify_boundary_info(aa_info, 1);
                bool aa_third_edge = triangles::classify_boundary_info(aa_info, 2);

//                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                           to_fixed(vertices[IND(ix + 0)].x, p), to_fixed(vertices[IND(ix + 0)].y, p),
                           to_fixed(vertices[IND(ix + 1)].x, p), to_fixed(vertices[IND(ix + 1)].y, p),
                           to_fixed(vertices[IND(ix + 2)].x, p), to_fixed(vertices[IND(ix + 2)].y, p),
                           opacity, p,
                           aa_first_edge, aa_second_edge, aa_third_edge
                           );
            }

            break;

        }
        case TrianglesIndices::TRIANGLES_FAN:

            for (index ix = 1; ix < size-1; ++ix) {

                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                        to_fixed(vertices[IND(0)].x, p), to_fixed(vertices[IND(0)].y, p),
                        to_fixed(vertices[IND(ix)].x, p), to_fixed(vertices[IND(ix)].y, p),
                        to_fixed(vertices[IND(ix + 1)].x, p), to_fixed(vertices[IND(ix + 1)].y, p),
                        opacity, p
                        );

            }

            break;
        case TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY:
        {
            index idx_boundary=0;

            for (index ix = 1; ix < size-1; ++ix) {
                index first_index = 0;
                index second_index = ix;
                index third_index = ix+1;
                boundary_info aa_info = boundary_buffer[idx_boundary++];

                bool aa_first_edge = triangles::classify_boundary_info(aa_info, 0);
                bool aa_second_edge = triangles::classify_boundary_info(aa_info, 1);
                bool aa_third_edge = triangles::classify_boundary_info(aa_info, 2);

                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                        to_fixed(vertices[IND(first_index)].x, p),  to_fixed(vertices[IND(first_index)].y, p),
                        to_fixed(vertices[IND(second_index)].x, p), to_fixed(vertices[IND(second_index)].y, p),
                        to_fixed(vertices[IND(third_index)].x, p),  to_fixed(vertices[IND(third_index)].y, p),
                        opacity, p,
                        aa_first_edge, aa_second_edge, aa_third_edge
                        );

            }

            break;
        }
        case TrianglesIndices::TRIANGLES_STRIP:
        {
            bool even = true;

            for (index ix = 0; ix < size-2; ++ix) {
                // we alternate order inorder to preserve CCW or CW,
                // in the future I will add face culling, which will
                // support only CW or CCW orientation_t at a time.

                index first_index = even ?  IND(ix + 0) : IND(ix + 2);
                index second_index = even ? IND(ix + 1) : IND(ix + 1);
                index third_index = even ?  IND(ix + 2) : IND(ix + 0);

                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                        to_fixed(vertices[first_index].x, p), to_fixed(vertices[first_index].y, p),
                        to_fixed(vertices[second_index].x, p), to_fixed(vertices[second_index].y, p),
                        to_fixed(vertices[third_index].x, p), to_fixed(vertices[third_index].y, p),
                        opacity, p
                        );

                even = !even;
            }

            break;
        }
        case TrianglesIndices::TRIANGLES_STRIP_WITH_BOUNDARY:
        {
            bool even = true;
            index idx_boundary = 0;

            for (index ix = 0; ix < size-2; ++ix) {
                // we alternate order inorder to preserve CCW or CW,
                // in the future I will add face culling, which will
                // support only CW or CCW orientation_t at a time.

                boundary_info aa_info = boundary_buffer[idx_boundary++];

                bool aa_first_edge = triangles::classify_boundary_info(aa_info, 0);
                bool aa_second_edge = triangles::classify_boundary_info(aa_info, 1);
                bool aa_third_edge = triangles::classify_boundary_info(aa_info, 2);

                index first_index = even ?  IND(ix + 0) : IND(ix + 2);
                index second_index = IND(ix + 1);
                index third_index = even ?  IND(ix + 2) : IND(ix + 0);

//                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                drawTriangle<BlendMode, PorterDuff, antialias>(color,
                        to_fixed(vertices[first_index].x, p), to_fixed(vertices[first_index].y, p),
                        to_fixed(vertices[second_index].x, p), to_fixed(vertices[second_index].y, p),
                        to_fixed(vertices[third_index].x, p), to_fixed(vertices[third_index].y, p),
                        opacity, p,
                        aa_first_edge, aa_second_edge, aa_third_edge
                        );

                even = !even;
            }

            break;
        }

    }

#undef IND
#undef to_fixed

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTrianglesWireframe(const color_f_t &color,
                                              const vec2<number> *vertices,
                                              const index *indices,
                                              const index size,
                                              const TrianglesIndices type,
                                              const opacity opacity) {

#define IND(a) indices[(a)]

    switch (type) {
        case TrianglesIndices::TRIANGLES:
        case TrianglesIndices::TRIANGLES_WITH_BOUNDARY:

            for (index ix = 0; ix < size; ix+=3) {

                drawTriangleWireframe(color,
                                      vertices[IND(ix + 0)],
                                      vertices[IND(ix + 1)],
                                      vertices[IND(ix + 2)]);
            }

            break;
        case TrianglesIndices::TRIANGLES_FAN:
        case TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY:

            for (index ix = 1; ix < size-1; ++ix) {

                drawTriangleWireframe(color,
                                      vertices[IND(0)],
                                      vertices[IND(ix)],
                                      vertices[IND(ix + 1)]);
            }

            break;

        case TrianglesIndices::TRIANGLES_STRIP:
        case TrianglesIndices::TRIANGLES_STRIP_WITH_BOUNDARY:
        {
            bool even = true;

            for (index ix = 0; ix < size-2; ++ix) {
                // we alternate order inorder to preserve CCW or CW,
                // in the future I will add face culling, which will
                // support only CW or CCW orientation_t at a time.
                if(even)
                    drawTriangleWireframe(color,
                                          vertices[IND(ix + 0)],
                                          vertices[IND(ix + 1)],
                                          vertices[IND(ix + 2)]);

                else
                    drawTriangleWireframe(color,
                                          vertices[IND(ix + 2)],
                                          vertices[IND(ix + 1)],
                                          vertices[IND(ix + 0)]);

                even = !even;
            }

            break;
        }

    }

#undef IND
}


template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTriangleWireframe(const color_f_t &color,
                                             const vec2<number> &p0,
                                             const vec2<number> &p1,
                                             const vec2<number> &p2)
{
    drawLine(color, p0.x, p0.y, p1.x, p1.y);
    drawLine(color, p1.x, p1.y, p2.x, p2.y);
    drawLine(color, p2.x, p2.y, p0.x, p0.y);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawTriangle(const color_f_t &color,
                                    int v0_x, int v0_y,
                                    int v1_x, int v1_y,
                                    int v2_x, int v2_y,
                                    const opacity opacity,
                                    const precision sub_pixel_precision,
                                    bool aa_first_edge,
                                    bool aa_second_edge,
                                    bool aa_third_edge) {

    fixed_signed sign = functions::orient2d({v0_x, v0_y},
            {v1_x, v1_y}, {v2_x, v2_y},
            sub_pixel_precision);
    // discard degenerate triangle
    if(sign==0)
        return;
    // convert CCW to CW triangle
    if(sign<0) {
        functions::swap(v1_x, v2_x);
        functions::swap(v1_y, v2_y);
        functions::swap(aa_first_edge, aa_third_edge);
    }

    color_t color_int;
    coder()->convert(color, color_int);
    bool perform_opacity = opacity < _max_alpha_value;
    // sub_pixel_precision;
    // THIS MAY HAVE TO BE MORE LIKE 15 TO AVOID OVERFLOW
    uint8_t MAX_BITS_FOR_PROCESSING_PRECISION = 15;
    uint8_t PR = MAX_BITS_FOR_PROCESSING_PRECISION;// - sub_pixel_precision;
    int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    // anti-alias pad for distance calculation
    uint8_t bits_distance = 0;
    uint8_t bits_distance_complement = 8;
    // max distance to consider in scaled space
    int max_distance_scaled_space_anti_alias=0;
    // we now decide which edges we want to anti-alias
    bool aa_all_edges=false;
    if(antialias) {
        aa_all_edges = aa_first_edge && aa_second_edge && aa_third_edge;

        bits_distance = 0;
        bits_distance_complement = 8 - bits_distance;
        // max distance to consider in canvas space
        int max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias << (PR);
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
        // minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
        // maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // bbox
//    max_sub_pixel_precision_value = 0;
    int minX = (functions::min(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int minY = (functions::min(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxX = (functions::max(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxY = (functions::max(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;

    // fill rules adjustments
    triangles::top_left_t top_left =
            triangles::classifyTopLeftEdges(false,
                                            v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);

    int bias_w0 = top_left.first  ? 0 : -1;
    int bias_w1 = top_left.second ? 0 : -1;
    int bias_w2 = top_left.third  ? 0 : -1;
    //
    const int block = 8;
//    minX -= block;
//    minY -= block;
//    maxX += block;
//    maxY += block;

//    minX &= ~(block - 1);
//    minY &= ~(block - 1);
//    maxX &= ~(block - 1);
//    maxY &= ~(block - 1);

    minX = functions::max(0, minX); minY = functions::max(0, minY);
    maxX = functions::min((width()), maxX); maxY = functions::min((height()), maxY);

    // Triangle setup
    int A01 = v0_y - v1_y, B01 = v1_x - v0_x;
    int A12 = v1_y - v2_y, B12 = v2_x - v1_x;
    int A20 = v2_y - v0_y, B20 = v0_x - v2_x;

    int A01_block = A01*block, B01_block = B01*block;
    int A12_block = A12*block, B12_block = B12*block;
    int A20_block = A20*block, B20_block = B20*block;

    int A01_block_m_1 = A01_block - A01, B01_block_m_1 = B01_block - B01;
    int A12_block_m_1 = A12_block - A12, B12_block_m_1 = B12_block - B12;
    int A20_block_m_1 = A20_block - A20, B20_block_m_1 = B20_block - B20;

    // Barycentric coordinates at minX/minY corner
    vec2_32i p_fixed = {  minX<<sub_pixel_precision, minY<<sub_pixel_precision };
    vec2_32i p = { minX , minY };

    int w0_row = (functions::orient2d(vec2_32i{v0_x, v0_y},vec2_32i{v1_x, v1_y},
                                      p_fixed, sub_pixel_precision) + bias_w0);
    int w1_row = (functions::orient2d(vec2_32i{v1_x, v1_y}, vec2_32i{v2_x, v2_y},
                                      p_fixed, sub_pixel_precision) + bias_w1);
    int w2_row = (functions::orient2d(vec2_32i{v2_x, v2_y}, vec2_32i{v0_x, v0_y},
                                      p_fixed, sub_pixel_precision) + bias_w2);

    // AA, 2A/L = h, therefore the division produces a P bit number
    int w0_row_h=0, w1_row_h=0, w2_row_h=0;
    int A01_h=0, B01_h=0, A12_h=0, B12_h=0, A20_h=0, B20_h=0;
    int A01_block_h=0, B01_block_h=0, A12_block_h=0, B12_block_h=0, A20_block_h=0, B20_block_h=0;
    int A01_block_m_1_h=0, B01_block_m_1_h=0, A12_block_m_1_h=0, B12_block_m_1_h=0, A20_block_m_1_h=0, B20_block_m_1_h=0;
    if(antialias) {
        int PP = PR;

        // lengths of edges
        unsigned int length_w0 = functions::length({v0_x, v0_y}, {v1_x, v1_y}, 0);
        unsigned int length_w1 = functions::length({v1_x, v1_y}, {v2_x, v2_y}, 0);
        unsigned int length_w2 = functions::length({v0_x, v0_y}, {v2_x, v2_y}, 0);

        A01_h = (((int64_t)(v0_y - v1_y))<<(PP))/length_w0, B01_h = (((int64_t)(v1_x - v0_x))<<(PP))/length_w0;
        A12_h = (((int64_t)(v1_y - v2_y))<<(PP))/length_w1, B12_h = (((int64_t)(v2_x - v1_x))<<(PP))/length_w1;
        A20_h = (((int64_t)(v2_y - v0_y))<<(PP))/length_w2, B20_h = (((int64_t)(v0_x - v2_x))<<(PP))/length_w2;

        A01_block_h = A01_h*block, B01_block_h = B01_h*block;
        A12_block_h = A12_h*block, B12_block_h = B12_h*block;
        A20_block_h = A20_h*block, B20_block_h = B20_h*block;

        A01_block_m_1_h = A01_block_h - A01_h, B01_block_m_1_h = B01_block_h - B01_h;
        A12_block_m_1_h = A12_block_h - A12_h, B12_block_m_1_h = B12_block_h - B12_h;
        A20_block_m_1_h = A20_block_h - A20_h, B20_block_m_1_h = B20_block_h - B20_h;

        w0_row_h = (((int64_t)(w0_row))<<(PP))/length_w0;
        w1_row_h = (((int64_t)(w1_row))<<(PP))/length_w1;
        w2_row_h = (((int64_t)(w2_row))<<(PP))/length_w2;
    }

    // watch out for negative values
    int index = p.y * (_width);
    int w_t_b = _width*block;

    for (p.y = minY; p.y <= maxY; p.y+=block) {

        // Barycentric coordinates at start of row
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        int w0_h=0,w1_h=0,w2_h=0;
        if(antialias) {
            w0_h = w0_row_h;
            w1_h = w1_row_h;
            w2_h = w2_row_h;
        }

        for (p.x = minX; p.x <= maxX; p.x+=block) {

            // Corners of block
            // test block bbox against each edge
            int top_left_w0 = w0;
            int top_left_w1 = w1;
            int top_left_w2 = w2;
            // next set of rows for bottom tests
            int bottom_left_w0 = top_left_w0 + B01_block_m_1;
            int bottom_left_w1 = top_left_w1 + B12_block_m_1;
            int bottom_left_w2 = top_left_w2 + B20_block_m_1;

            int top_right_w0 = top_left_w0 + A01_block_m_1;
            int top_right_w1 = top_left_w1 + A12_block_m_1;
            int top_right_w2 = top_left_w2 + A20_block_m_1;

            int bottom_right_w0 = bottom_left_w0 + A01_block_m_1;
            int bottom_right_w1 = bottom_left_w1 + A12_block_m_1;
            int bottom_right_w2 = bottom_left_w2 + A20_block_m_1;

            bool w0_in = (top_left_w0 | top_right_w0 | bottom_right_w0 | bottom_left_w0)>=0;
            bool w1_in = (top_left_w1 | top_right_w1 | bottom_right_w1 | bottom_left_w1)>=0;
            bool w2_in = (top_left_w2 | top_right_w2 | bottom_right_w2 | bottom_left_w2)>=0;

            bool in = w0_in && w1_in && w2_in;

            if (in) {
                int stride = index;
                int block_start_x = functions::max(p.x, minX);
                int block_start_y = functions::max(p.y, minY);
                int block_end_x = functions::min(p.x + block, maxX);
                int block_end_y = functions::min(p.y + block, maxY);

                for(int iy = block_start_y; iy <block_end_y; iy++) {
                    for(int ix = block_start_x; ix < block_end_x; ix++)
                        blendColor<BlendMode, PorterDuff>(color_int, (stride + ix), opacity);

                    stride += _width;
                }

            }
            // we are on the outside or on the boundary
            else {
                bool w0_out = (top_left_w0 & top_right_w0 & bottom_right_w0 & bottom_left_w0)<0;
                bool w1_out = (top_left_w1 & top_right_w1 & bottom_right_w1 & bottom_left_w1)<0;
                bool w2_out = (top_left_w2 & top_right_w2 & bottom_right_w2 & bottom_left_w2)<0;

                bool out = w0_out || w1_out || w2_out;
                bool boundary = !out;

                // now test if block is also interesting for AA
                if(antialias && out) {
                    int top_left_w0_h = w0_h;
                    int top_left_w1_h = w1_h;
                    int top_left_w2_h = w2_h;
                    // next set of rows for bottom tests
                    int bottom_left_w0_h = top_left_w0_h + B01_block_m_1_h;
                    int bottom_left_w1_h = top_left_w1_h + B12_block_m_1_h;
                    int bottom_left_w2_h = top_left_w2_h + B20_block_m_1_h;

                    int top_right_w0_h = top_left_w0_h + A01_block_m_1_h;
                    int top_right_w1_h = top_left_w1_h + A12_block_m_1_h;
                    int top_right_w2_h = top_left_w2_h + A20_block_m_1_h;

                    int bottom_right_w0_h = bottom_left_w0_h + A01_block_m_1_h;
                    int bottom_right_w1_h = bottom_left_w1_h + A12_block_m_1_h;
                    int bottom_right_w2_h = bottom_left_w2_h + A20_block_m_1_h;

                    // distance of block to the edge w0
                    // since we are outside, all of the distances are negative, therefore
                    // taking max function on negatives reveals the closest distance
                    int distance_w0 = functions::min(abs(top_left_w0_h), abs(bottom_left_w0_h),
                                                     abs(top_right_w0_h), abs(bottom_right_w0_h));
                    int distance_w1 = functions::min(abs(top_left_w1_h), abs(bottom_left_w1_h),
                                                     abs(top_right_w1_h), abs(bottom_right_w1_h));
                    int distance_w2 = functions::min(abs(top_left_w2_h), abs(bottom_left_w2_h),
                                                     abs(top_right_w2_h), abs(bottom_right_w2_h));

                    // now take the minimum among absolute values of distances
                    int d_aa = functions::min((distance_w0), (distance_w1), (distance_w2));
                    // todo:: one bug I notices, what happens when pixel falls on
                    // todo:: a straight line ? this causes artifact !!!
                    if(true) {
                        int delta = -d_aa + max_distance_scaled_space_anti_alias;
                        boundary = boundary || delta>=0;
                    }
                }

                if(boundary) {
                    int stride = index;

                    int w0_row_ = w0;
                    int w1_row_ = w1;
                    int w2_row_ = w2;

                    int w0_row_h_,w1_row_h_,w2_row_h_;
                    if(antialias) {
                        w0_row_h_ = w0_h;
                        w1_row_h_ = w1_h;
                        w2_row_h_ = w2_h;
                    }

                    int block_start_x = functions::max(p.x, minX);
                    int block_start_y = functions::max(p.y, minY);
                    int block_end_x = functions::min(p.x + block, maxX);
                    int block_end_y = functions::min(p.y + block, maxY);

                    for (int iy = block_start_y; iy < block_end_y; iy++) {

                        int w0_ = w0_row_;
                        int w1_ = w1_row_;
                        int w2_ = w2_row_;

                        int w0_h_,w1_h_,w2_h_;
                        if(antialias) {
                            w0_h_ = w0_row_h_;
                            w1_h_ = w1_row_h_;
                            w2_h_ = w2_row_h_;
                        }

                        for (int ix = block_start_x; ix < block_end_x; ix++) {
                            if ((w0_ | w1_ | w2_) >= 0)
                                blendColor<BlendMode, PorterDuff>(color_int, (stride + ix), opacity);
                            else if(antialias) {
                                // if any of the distances are negative, we are outside.
                                // test if we can anti-alias
                                // take minimum of all meta distances

                                // find minimal distance along edges only, this does not take
                                // into account the junctions

                                int distance = functions::min((w0_h_), (w1_h_), (w2_h_));
                                int delta = ((distance) + max_distance_scaled_space_anti_alias);
                                bool perform_aa = aa_all_edges;


                                // test edges
                                if(!perform_aa) {
                                    if((distance==w0_h_) && aa_first_edge)
                                        perform_aa = true;
                                    else if((distance==w1_h_) && aa_second_edge)
                                        perform_aa = true;
                                    else perform_aa = (distance == w2_h_) && aa_third_edge;
                                }

                                {
                                    bool on_cusp = w0_h_<=0 && w1_h_<=0;
                                    on_cusp |= (w1_h_<=0 && w2_h_<=0);
                                    on_cusp |= (w2_h_<=0 && w0_h_<=0);
                                    perform_aa &= !(on_cusp);
                                }

                                if (perform_aa && delta>=0) {

                                    // take the complement and rescale
                                    uint8_t blend = functions::clamp<int>(((int64_t)delta << bits_distance_complement)>>(PR),
                                                                          0, 255);

                                    if (perform_opacity) {
                                        blend = (blend * opacity) >> 8;
                                    }

//                                    blendColor<BlendMode, PorterDuff>({0,0,0}, (stride + ix), 255);
                                    blendColor<BlendMode, PorterDuff>(color_int, (stride + ix), blend);
                                }

                            }

                            w0_ += A01;
                            w1_ += A12;
                            w2_ += A20;

                            if(antialias) {
                                w0_h_ += A01_h;
                                w1_h_ += A12_h;
                                w2_h_ += A20_h;
                            }

                        }

                        w0_row_ += B01;
                        w1_row_ += B12;
                        w2_row_ += B20;

                        if(antialias) {
                            w0_row_h_ += B01_h;
                            w1_row_h_ += B12_h;
                            w2_row_h_ += B20_h;
                        }

                        stride += _width;

                    }

                }

            }

            // One step to the right
            w0 += A01_block;
            w1 += A12_block;
            w2 += A20_block;

            if(antialias) {
                w0_h += A01_block_h;
                w1_h += A12_block_h;
                w2_h += A20_block_h;
            }

        }

        // One row step
        w0_row += B01_block;
        w1_row += B12_block;
        w2_row += B20_block;

        if(antialias) {
            w0_row_h += B01_block_h;
            w1_row_h += B12_block_h;
            w2_row_h += B20_block_h;
        }

        index += w_t_b;
    }

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTriangle(const color_f_t &color,
                                    const number v0_x, const number v0_y,
                                    const number v1_x, const number v1_y,
                                    const number v2_x, const number v2_y,
                                    const opacity opacity,
                                    bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    precision precision = 4;
    int v0_x_ = microgl::math::to_fixed(v0_x, precision);
    int v0_y_ = microgl::math::to_fixed(v0_y, precision);
    int v1_x_ = microgl::math::to_fixed(v1_x, precision);
    int v1_y_ = microgl::math::to_fixed(v1_y, precision);
    int v2_x_ = microgl::math::to_fixed(v2_x, precision);
    int v2_y_ = microgl::math::to_fixed(v2_y, precision);
    drawTriangle<BlendMode, PorterDuff, antialias>(color,
            v0_x_, v0_y_,
            v1_x_, v1_y_,
            v2_x_, v2_y_,
            opacity, precision,
            aa_first_edge, aa_second_edge, aa_third_edge);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, bool perspective_correct,
        typename Sampler,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               const int v0_x, const int v0_y, int u0, int v0, int q0,
                               const int v1_x, const int v1_y, int u1, int v1, int q1,
                               const int v2_x, const int v2_y, int u2, int v2, int q2,
                               const uint8_t opacity, const uint8_t sub_pixel_precision, const uint8_t uv_precision,
                               bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {

    fixed_signed area = functions::orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    //int bmp_width = bmp.width();

    // sub_pixel_precision;
    // THIS MAY HAVE TO BE MORE LIKE 15 TO AVOID OVERFLOW
    uint8_t BITS_UV_COORDS = uv_precision;
    uint8_t PREC_DIST = 15;

    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    // bounding box
    int minX = (functions::min(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int minY = (functions::min(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxX = (functions::max(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxY = (functions::max(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;

    // anti-alias pad for distance calculation
    uint8_t bits_distance = 0;
    uint8_t bits_distance_complement = 8;
    // max distance to consider in canvas space
    unsigned int max_distance_canvas_space_anti_alias=0;
    // max distance to consider in scaled space
    unsigned int max_distance_scaled_space_anti_alias=0;

    bool aa_all_edges = false;
    if(antialias) {
        aa_all_edges = aa_first_edge && aa_second_edge && aa_third_edge;

        bits_distance = 0;
        bits_distance_complement = 8 - bits_distance;
        max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias<<PREC_DIST;
    }

    // fill rules adjustments
    triangles::top_left_t top_left =
            triangles::classifyTopLeftEdges(false,
                                            v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);

    int bias_w0 = top_left.first  ? 0 : -1;
    int bias_w1 = top_left.second ? 0 : -1;
    int bias_w2 = top_left.third  ? 0 : -1;
    //

    // clipping
    minX = functions::max(0, minX); minY = functions::max(0, minY);
    maxX = functions::min(width()-1, maxX); maxY = functions::min(height()-1, maxY);

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };
    vec2_32i p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };

    int bmp_w_max = bmp.width() - 1, bmp_h_max = bmp.height() - 1;

    // this can produce a 2P bits number if the points form a a perpendicular triangle
    fixed_signed area_v1_v2_p = functions::orient2d({v1_x, v1_y}, {v2_x, v2_y}, p_fixed, sub_pixel_precision) + bias_w1,
            area_v2_v0_p = functions::orient2d({v2_x, v2_y}, {v0_x, v0_y}, p_fixed, sub_pixel_precision) + bias_w2,
            area_v0_v1_p = functions::orient2d({v0_x, v0_y}, {v1_x, v1_y}, p_fixed, sub_pixel_precision) + bias_w0;

    uint8_t MAX_PREC = 60;
    uint8_t LL = MAX_PREC - (sub_pixel_precision + BITS_UV_COORDS);
    uint64_t ONE = ((uint64_t)1)<<LL;
    uint64_t one_area = (ONE) / area;

    // PR seems very good for the following calculations
    // Triangle setup
    // this needs at least (P+1) bits, since the delta is always <= length
    int A01 = (v0_y - v1_y), B01 = (v1_x - v0_x);
    int A12 = (v1_y - v2_y), B12 = (v2_x - v1_x);
    int A20 = (v2_y - v0_y), B20 = (v0_x - v2_x);

    int w0_row = (area_v0_v1_p);
    int w1_row = (area_v1_v2_p);
    int w2_row = (area_v2_v0_p);

    // AA, 2A/L = h, therefore the division produces a P bit number
    int w0_row_h=0, w1_row_h=0, w2_row_h=0;
    int A01_h=0, B01_h=0, A12_h=0, B12_h=0, A20_h=0, B20_h=0;

    if(antialias) {
        // lengths of edges, produces a P+1 bits number
        unsigned int length_w0 = functions::length({v0_x, v0_y}, {v1_x, v1_y}, 0);
        unsigned int length_w1 = functions::length({v1_x, v1_y}, {v2_x, v2_y}, 0);
        unsigned int length_w2 = functions::length({v0_x, v0_y}, {v2_x, v2_y}, 0);

        A01_h = ((int64_t)(v0_y - v1_y)<<PREC_DIST)/length_w0, B01_h = ((int64_t)(v1_x - v0_x)<<PREC_DIST)/length_w0;
        A12_h = ((int64_t)(v1_y - v2_y)<<PREC_DIST)/length_w1, B12_h = ((int64_t)(v2_x - v1_x)<<PREC_DIST)/length_w1;
        A20_h = ((int64_t)(v2_y - v0_y)<<PREC_DIST)/length_w2, B20_h = ((int64_t)(v0_x - v2_x)<<PREC_DIST)/length_w2;

        w0_row_h = ((int64_t)(area_v0_v1_p)<<PREC_DIST)/length_w0;
        w1_row_h = ((int64_t)(area_v1_v2_p)<<PREC_DIST)/length_w1;
        w2_row_h = ((int64_t)(area_v2_v0_p)<<PREC_DIST)/length_w2;
    }

    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        int w0_h=0,w1_h=0,w2_h=0;

        if(antialias) {
            w0_h = w0_row_h;
            w1_h = w1_row_h;
            w2_h = w2_row_h;
        }

        for (p.x = minX; p.x <= maxX; p.x++) {

            if ((w0 | w1 | w2) >= 0) {

                int u_i, v_i;
                uint64_t u_fixed = (((uint64_t)((uint64_t)w0*u2 + (uint64_t)w1*u0 + (uint64_t)w2*u1)));
                uint64_t v_fixed = (((uint64_t)((uint64_t)w0*v2 + (uint64_t)w1*v0 + (uint64_t)w2*v1)));

                if(perspective_correct) {
                    uint64_t q_fixed =(((uint64_t)((uint64_t)w0*q2 + (uint64_t)w1*q0 + (uint64_t)w2*q1)));
                    uint64_t one_over_q = ONE / q_fixed;

                    u_i = (u_fixed*bmp_w_max*one_over_q)>>(LL-BITS_UV_COORDS);
                    v_i = (v_fixed*bmp_h_max*one_over_q)>>(LL-BITS_UV_COORDS);

                } else {

                    u_fixed = ((u_fixed*one_area)>>(LL - BITS_UV_COORDS));
                    v_fixed = ((v_fixed*one_area)>>(LL - BITS_UV_COORDS));
                    // coords in :BITS_UV_COORDS space
                    u_i = (bmp_w_max*u_fixed)>>(BITS_UV_COORDS);
                    v_i = (bmp_h_max*v_fixed)>>(BITS_UV_COORDS);
                }

                //u_i = functions::clamp<int>(u_i, 0, bmp_w_max<<BITS_UV_COORDS);
                //v_i = functions::clamp<int>(v_i, 0, bmp_h_max<<BITS_UV_COORDS);

                color_t col_bmp;
                //bmp.decode(index_bmp, col_bmp);
                Sampler::sample(bmp, u_i, v_i, BITS_UV_COORDS, col_bmp);

                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

            } else if(antialias) {
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = functions::min(w0_h, w1_h, w2_h);
                int delta = (distance) + max_distance_scaled_space_anti_alias;
                bool perform_aa = aa_all_edges;

                // test edges
                if(!perform_aa) {
                    if(distance==w0_h && aa_first_edge)
                        perform_aa = true;
                    else if(distance==w1_h && aa_second_edge)
                        perform_aa = true;
                    else perform_aa = distance == w2_h && aa_third_edge;
                }

                if (perform_aa && delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary
                    // I don't round since I don't care about it here

                    int u_i, v_i;
                    uint64_t u_fixed = (((uint64_t)((uint64_t)w0*u2 + (uint64_t)w1*u0 + (uint64_t)w2*u1)));
                    uint64_t v_fixed = (((uint64_t)((uint64_t)w0*v2 + (uint64_t)w1*v0 + (uint64_t)w2*v1)));

                    if(perspective_correct) {

                        uint64_t q_fixed =(((uint64_t)((uint64_t)w0*q2 + (uint64_t)w1*q0 + (uint64_t)w2*q1)));
                        uint64_t one_over_q = ONE / q_fixed;

                        u_i = (u_fixed*bmp_w_max*one_over_q)>>(LL-BITS_UV_COORDS);
                        v_i = (v_fixed*bmp_h_max*one_over_q)>>(LL-BITS_UV_COORDS);

                    } else {

                        u_fixed = ((u_fixed*one_area)>>(LL - BITS_UV_COORDS));
                        v_fixed = ((v_fixed*one_area)>>(LL - BITS_UV_COORDS));
                        // coords in :BITS_UV_COORDS space
                        u_i = (bmp_w_max*u_fixed)>>(BITS_UV_COORDS);
                        v_i = (bmp_h_max*v_fixed)>>(BITS_UV_COORDS);
                    }

                    u_i = functions::clamp<int>(u_i, 0, bmp_w_max<<BITS_UV_COORDS);
                    v_i = functions::clamp<int>(v_i, 0, bmp_h_max<<BITS_UV_COORDS);

                    color_t col_bmp;
                    Sampler::sample(bmp, u_i, v_i, BITS_UV_COORDS, col_bmp);
                    // complement and normalize
                    uint8_t blend = functions::clamp<int>(((uint64_t)(delta << bits_distance_complement))>>PREC_DIST,
                                                          0, 255);

                    if (opacity < _max_alpha_value)
                        blend = (blend * opacity) >> 8;

                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }

            w0 += A01;
            w1 += A12;
            w2 += A20;

            if(antialias) {
                w0_h += A01_h;
                w1_h += A12_h;
                w2_h += A20_h;
            }

        }

        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        if(antialias) {
            w0_row_h += B01_h;
            w1_row_h += B12_h;
            w2_row_h += B20_h;
        }

        index += _width;
    }

}

// todo
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename Sampler,
        typename P2, typename CODER2, typename number>
void
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               const number v0_x, const number v0_y, number u0, number v0,
                               const number v1_x, const number v1_y, number u1, number v1,
                               const number v2_x, const number v2_y, number u2, number v2,
                               const uint8_t opacity,
                               bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {

    precision prec_pixel = 4;
    precision prec_uv = 5;
    int v0_x_ = microgl::math::to_fixed(v0_x, prec_pixel);
    int v0_y_ = microgl::math::to_fixed(v0_y, prec_pixel);
    int v1_x_ = microgl::math::to_fixed(v1_x, prec_pixel);
    int v1_y_ = microgl::math::to_fixed(v1_y, prec_pixel);
    int v2_x_ = microgl::math::to_fixed(v2_x, prec_pixel);
    int v2_y_ = microgl::math::to_fixed(v2_y, prec_pixel);

    int u0_ = microgl::math::to_fixed(u0, prec_uv);
    int v0_ = microgl::math::to_fixed(v0, prec_uv);
    int u1_ = microgl::math::to_fixed(u1, prec_uv);
    int v1_ = microgl::math::to_fixed(v1, prec_uv);
    int u2_ = microgl::math::to_fixed(u2, prec_uv);
    int v2_ = microgl::math::to_fixed(v2, prec_uv);
    int q_ = microgl::math::to_fixed(number(1), prec_uv);

    drawTriangle<BlendMode, PorterDuff, antialias, false, Sampler>(bmp,
            v0_x_, v0_y_, u0_, v0_, q_,
            v1_x_, v1_y_, u1_, v1_, q_,
            v2_x_, v2_y_, u2_, v2_, q_,
            opacity, prec_pixel, prec_uv,
            aa_first_edge, aa_second_edge, aa_third_edge);

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, typename Sampler, typename number,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawQuadrilateral(const Bitmap<P2, CODER2> & bmp,
                                    number v0_x, number v0_y, number u0, number v0,
                                    number v1_x, number v1_y, number u1, number v1,
                                    number v2_x, number v2_y, number u2, number v2,
                                    number v3_x, number v3_y, number u3, number v3,
                                    const uint8_t opacity) {
    precision uv_p = 8;
    precision pixel_p = 4;

#define f microgl::math::to_fixed

    number q0 = 1, q1 = 1, q2 = 1, q3 = 1;
    number one(1), zero(0);
    number p0x = v0_x; number p0y = v0_y;
    number p1x = v1_x; number p1y = v1_y;
    number p2x = v2_x; number p2y = v2_y;
    number p3x = v3_x; number p3y = v3_y;

    number ax = p2x - p0x;
    number ay = p2y - p0y;
    number bx = p3x - p1x;
    number by = p3y - p1y;
    number t, s;
    number cross = ax * by - ay * bx;

    if (cross != zero) {
        number cy = p0y - p1y;
        number cx = p0x - p1x;

        s = number(ax * cy - ay * cx) / cross;
        if (s > zero && s < one) {
            t = number(bx * cy - by * cx) / cross;

            if (t > zero && t < one) {

                q0 = one / (one - t);
                q1 = one / (one - s);
                q2 = one / t;
                q3 = one / s;

            }
        }
    }

    number u0_q0 = u0*q0, v0_q0 = v0*q0;
    number u1_q1 = u1*q1, v1_q1 = v1*q1;
    number u2_q2 = u2*q2, v2_q2 = v2*q2;
    number u3_q3 = u3*q3, v3_q3 = v3*q3;

    // perspective correct version
    drawTriangle<BlendMode, PorterDuff, antialias, true, Sampler>(bmp,
          f(v0_x, pixel_p), f(v0_y, pixel_p), f(u0_q0, uv_p), f(v0_q0, uv_p), f(q0, uv_p),
          f(v1_x, pixel_p), f(v1_y, pixel_p), f(u1_q1, uv_p), f(v1_q1, uv_p), f(q1, uv_p),
          f(v2_x, pixel_p), f(v2_y, pixel_p), f(u2_q2, uv_p), f(v2_q2, uv_p), f(q2, uv_p),
          opacity, pixel_p,
          uv_p,
          true, true, false);

    drawTriangle<BlendMode, PorterDuff, antialias, true, Sampler>(bmp,
          f(v2_x, pixel_p), f(v2_y, pixel_p), f(u2_q2, uv_p), f(v2_q2, uv_p), f(q2, uv_p),
          f(v3_x, pixel_p), f(v3_y, pixel_p), f(u3_q3, uv_p), f(v3_q3, uv_p), f(q3, uv_p),
          f(v0_x, pixel_p), f(v0_y, pixel_p), f(u0_q0, uv_p), f(v0_q0, uv_p), f(q0, uv_p),
          opacity, pixel_p,
          uv_p,
          true, true, false);

#undef f

}

// quads

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
void Canvas<P, CODER>::drawQuad(const color_f_t & color,
                                const int left, const int top,
                                const int right, const int bottom,
                                const precision sub_pixel_precision,
                                const opacity opacity) {
    color_t color_int;
    this->coder()->convert(color, color_int);

    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    int left_ = functions::max((left + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)0);
    int top_ = functions::max((top + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)0);
    int right_ = functions::min((right + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)width());
    int bottom_ = functions::min((bottom + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)height());

    int index = top_ * _width;
    for (int y = top_; y < bottom_; y++) {
        for (int x = left_; x < right_; x++) {
            blendColor<BlendMode, PorterDuff>(color_int, index + x, opacity);
        }

        index += _width;
    }

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, typename number>
void Canvas<P, CODER>::drawQuad(const color_f_t & color,
                                const number left, const number top,
                                const number right, const number bottom,
                                const opacity opacity) {
    uint8_t p = 4;

    drawQuad(color,
             microgl::math::to_fixed(left, p), microgl::math::to_fixed(top, p),
             microgl::math::to_fixed(right, p), microgl::math::to_fixed(bottom, p),
             p, opacity
    );

}

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff,
        typename Sampler,
        typename P2, typename CODER2>
void Canvas<P, CODER>::drawQuad(const Bitmap<P2, CODER2> &bmp,
                                const int left, const int top,
                                const int right, const int bottom,
                                const int u0, const int v0,
                                const int u1, const int v1,
                                const precision sub_pixel_precision,
                                const precision uv_precision,
                                const opacity opacity) {
    color_t col_bmp{};
    P converted{};

    uint8_t DIV_prec = 16;
    uint8_t DIV_prec_minus_sub_pixel = DIV_prec - sub_pixel_precision;
    // if you are using half, don't forget to clamp down the road,
    // but it will take cycles so I don't do it !!!
    int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    int bmp_width = bmp.width();
    int bmp_height = bmp.height();
    int bmp_w_max = bmp_width - 1;
    int bmp_h_max = bmp_height - 1;

    int left_ = functions::max((left + max_sub_pixel_precision_value) >> sub_pixel_precision, (int)0);
    int top_ = functions::max((top + max_sub_pixel_precision_value) >> sub_pixel_precision, ( int)0);
    int right_ = functions::min((right + max_sub_pixel_precision_value) >> sub_pixel_precision, ( int)width());
    int bottom_ = functions::min((bottom + max_sub_pixel_precision_value) >> sub_pixel_precision, ( int)height());

    int ddu = int_to_fixed_2(((u1-u0)*bmp_width)>>uv_precision, DIV_prec);
    int ddv = int_to_fixed_2((-(v1-v0)*bmp_height)>>uv_precision, DIV_prec);

    int max_uv = (1<<uv_precision);
    int u_start = int_to_fixed_2((u0*bmp_w_max)>>uv_precision, DIV_prec_minus_sub_pixel);
    // this is more stable to step forward than backward
    int v_start = int_to_fixed_2(((max_uv-v0)*bmp_h_max)>>uv_precision, DIV_prec_minus_sub_pixel);
    int du = (right-left)==0 ? 0 : fixed_div_int(ddu, right-left);
    int dv = (bottom-top)==0 ? 0 : fixed_div_int(ddv, bottom-top);
    int u = u_start, v = v_start;

    int index;

    index = top_ * _width;

    v = bmp_h_max<<DIV_prec_minus_sub_pixel;

    for (int y = top_; y < bottom_; y++) {

        for (int x = left_; x < right_; x++) {
            Sampler::sample(bmp, u, v,
                            DIV_prec_minus_sub_pixel,
                            col_bmp);

            // compile time branching
//            if(Sampler::type() != sampler::type::NearestNeighbor)
//                Sampler::sample(bmp, u, v,
//                        DIV_prec_minus_sub_pixel, col_bmp);
//            else {
//                u_i = fixed_to_int_2(u + f_half, DIV_prec_minus_sub_pixel);
//                index_bmp = (v_i) + u_i;
//                // decode the bitmap
//                bmp.decode(index_bmp, col_bmp);
//            }
            //

            // re-encode for a different canvas
            blendColor<BlendMode, PorterDuff>(col_bmp, index + x, opacity);

            u += du;
        }

        u = u_start;
        v += dv;
        index += _width;
    }

}

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff, typename Sampler,
        typename P2, typename CODER2, typename number>
void Canvas<P, CODER>::drawQuad(const Bitmap<P2, CODER2> &bmp,
                                const number left, const number top,
                                const number right, const number bottom,
                                const number u0, const number v0,
                                const number u1, const number v1,
                                const opacity opacity) {
    precision p_sub = 4;
    precision p_uv = 5;

    drawQuad<BlendMode, PorterDuff, Sampler>(bmp,
            microgl::math::to_fixed(left, p_sub), microgl::math::to_fixed(top, p_sub),
            microgl::math::to_fixed(right, p_sub), microgl::math::to_fixed(bottom, p_sub),
            microgl::math::to_fixed(u0, p_uv), microgl::math::to_fixed(v0, p_uv),
            microgl::math::to_fixed(u1, p_uv), microgl::math::to_fixed(v1, p_uv),
            p_sub, p_uv, opacity
            );
}

template<typename P, typename CODER>
template<typename number>
void Canvas<P, CODER>::drawLine(const color_f_t &color,
                                number x0, number y0,
                                number x1, number y1) {
    precision p = 4;
    int x0_ = microgl::math::to_fixed(x0, p);
    int y0_ = microgl::math::to_fixed(y0, p);
    int x1_ = microgl::math::to_fixed(x1, p);
    int y1_ = microgl::math::to_fixed(y1, p);
    drawLine(color, x0_, y0_, x1_, y1_, p);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawLine(const color_f_t &color,
                                int x0, int y0,
                                int x1, int y1,
                                precision bits) {

    int X0 = x0, Y0 = y0, X1 = x1, Y1=y1;
    color_t color_input{};

    coder()->convert(color, color_input);

    unsigned int IntensityBits = 8;
    unsigned int NumLevels = 1 << IntensityBits;
    unsigned int maxIntensity = NumLevels - 1;
    uint32_t IntensityShift, ErrorAdj, ErrorAcc;
    unsigned int ErrorAccTemp, Weighting, WeightingComplementMask;
    int DeltaX, DeltaY, Temp, XDir;//, YDir;
    unsigned int one = 1u<<bits;
    unsigned int round = 0;//one>>1;// -1;
    // Make sure the line runs top to bottom
    if (Y0 > Y1) {
        Temp = Y0; Y0 = Y1; Y1 = Temp;
        Temp = X0; X0 = X1; X1 = Temp;
    }

    // Draw the initial pixel, which is always exactly intersected by
    // the line and so needs no weighting
    blendColor(color_input, (X0+round)>>bits, (Y0+round)>>bits, maxIntensity);

    if ((DeltaX = X1 - X0) >= 0) {
        XDir = 1u<<bits;
    } else {
        XDir = -(1u<<bits);
        DeltaX = -DeltaX; // make DeltaX positive
    }
    DeltaY = Y1 - Y0;

    // Special-case horizontal, vertical, and diagonal lines, which
    // require no weighting because they go right through the center of
    // every pixel
    if ((Y1 - Y0) == 0) {
        // Horizontal line
        while ((DeltaX-=one) > 0) {
            X0 += XDir;
            blendColor(color_input, X0>>bits, Y0>>bits, maxIntensity);

        }
        return;
    }
    if (DeltaX == 0) {
        // Vertical line
        do {
            Y0+=one;
            blendColor(color_input, X0>>bits, Y0>>bits, maxIntensity);
        } while ((DeltaY-=one) > 0);
        return;
    }
    if (DeltaX == DeltaY) {
        // Diagonal line
        do {
            X0 += XDir;
            Y0+=one;
            blendColor(color_input, X0>>bits, Y0>>bits, maxIntensity);
        } while ((DeltaY-=one) > 0);
        return;
    }

    // line is not horizontal, diagonal, or vertical
    ErrorAcc = 0; // initialize the line error accumulator to 0
    // # of bits by which to shift ErrorAcc to get intensity level
    IntensityShift = 32 - IntensityBits;
    // Mask used to flip all bits in an intensity weighting, producing the
    // result (1 - intensity weighting)
    WeightingComplementMask = maxIntensity;

    // Is this an X-major or Y-major line?
    if (DeltaY > DeltaX) {
        // Y-major line; calculate 16-bit fixed-point fractional part of a
        // pixel that X advances each time Y advances 1 pixel, truncating the
        // result so that we won't overrun the endpoint along the X axis
        ErrorAdj = ((unsigned long long) DeltaX << 32) / (unsigned long long) DeltaY;
        // Draw all pixels other than the first and last
        while ((DeltaY-=one) > 0) {
            ErrorAccTemp = ErrorAcc; // remember currrent accumulated error
            ErrorAcc += ErrorAdj; // calculate error for next pixel
            if (ErrorAcc <= ErrorAccTemp) {
                // The error accumulator turned over, so advance the X coord
                X0 += XDir;
            }
            Y0+=one; // Y-major, so always advance Y
            // The IntensityBits most significant bits of ErrorAcc give us the
            // intensity weighting for this pixel, and the complement of the
            // weighting for the paired pixel
            Weighting = ErrorAcc >> IntensityShift;

            unsigned int mix = (Weighting ^ WeightingComplementMask);
            // this equals Weighting, but I write it like that for clarity for now
            unsigned int mix_complement = maxIntensity - mix;

            blendColor(color_input, X0>>bits, Y0>>bits, mix);
            blendColor(color_input, (X0 + XDir)>>bits, Y0>>bits, mix_complement);
        }

        // Draw the final pixel, which is always exactly intersected by the line
        // and so needs no weighting
        blendColor(color_input, (X1+round)>>bits, (Y1+round)>>bits, maxIntensity);
        return;
    }

    // It's an X-major line; calculate 16-bit fixed-point fractional part of a
    // pixel that Y advances each time X advances 1 pixel, truncating the
    // result to avoid overrunning the endpoint along the X axis
    ErrorAdj = ((unsigned long long) DeltaY << 32) / (unsigned long long) DeltaX;

    // Draw all pixels other than the first and last
    while ((DeltaX-=one) > 0) {
        ErrorAccTemp = ErrorAcc; // remember currrent accumulated error
        ErrorAcc += ErrorAdj; // calculate error for next pixel
        if (ErrorAcc <= ErrorAccTemp) {
            // The error accumulator turned over, so advance the Y coord
            Y0+=one;
        }
        X0 += XDir; // X-major, so always advance X

        // The IntensityBits most significant bits of ErrorAcc give us the
        // intensity weighting for this pixel, and the complement of the
        // weighting for the paired pixel
        Weighting = (ErrorAcc >> IntensityShift);

        // Tomer notes:
        // 1. i inverted the order because i do not use palettes like Michael.
        // 2. we can halve the multiplications, but add more verbosity and unreadable code
        //
        unsigned int mix = (Weighting ^ WeightingComplementMask);
        unsigned int mix_complement = maxIntensity - mix; // this equals Weighting

        blendColor(color_input, X0>>bits, Y0>>bits, mix);
        blendColor(color_input, X0>>bits, (Y0 + one)>>bits, mix_complement);
    }

    // Draw the final pixel, which is always exactly intersected by the line
    // and so needs no weighting
    blendColor(color_input, (X1+round)>>bits, (Y1+round)>>bits, maxIntensity);
}

// todo: drawLinePath will be removed once the path maker is ready
template<typename P, typename CODER>
void
Canvas<P, CODER>::drawLinePath(color_f_t &color,
                               vec2_32i *points,
                               precision precision,
                               unsigned int size,
                               bool closed_path) {
    index jx = 0;
    for (jx = 0; jx < size; jx++) {

        if(jx)
            drawLine(color, points[jx-1].x, points[jx-1].y, points[jx].x, points[jx].y, precision);
    }

    if(closed_path)
        drawLine(color, points[0].x, points[0].y, points[jx - 1].x, points[jx- 1].y, precision);

}

template<typename P, typename CODER>
void
Canvas<P, CODER>::drawLinePath(color_f_t &color,
                               vec2_f *points,
                               unsigned int size,
                               bool closed_path) {

    uint8_t p = 0;
    int jx = 0;

    for (jx = 0; jx < size; jx++) {

        if(jx)
            drawLine(color,
                     float_to_fixed_2(points[jx-1].x, p), float_to_fixed_2(points[jx-1].y, p),
                     float_to_fixed_2(points[jx].x, p), float_to_fixed_2(points[jx].y, p),
                     p);

    }

    if(closed_path)
        drawLine(color, points[0].x, points[0].y, points[jx - 1].x, points[jx- 1].y, p);

}

// todo: drawBezierPath will be removed once the path maker is ready

template<typename P, typename CODER>
template<typename number>
void Canvas<P, CODER>::drawBezierPath(color_f_t & color, vec2<number> *points,
                                               unsigned int size,
                                               typename tessellation::curve_divider<number>::Type type,
                                               typename tessellation::curve_divider<number>::CurveDivisionAlgorithm algorithm) {
    using vertex = vec2<number>;
    dynamic_array<vertex> samples;
    vertex previous, current;
    using c = tessellation::curve_divider<number>;
    index pitch = type==c::Type::Quadratic ? 2 : 3;
    number circle_diameter = 5;
    for (index jx = 0; jx < size-pitch; jx+=pitch) {
        auto * point_anchor = &points[jx];

        samples.clear();

        c::compute(point_anchor, samples, algorithm, type);

        for (index ix = 0; ix < samples.size(); ++ix) {
            current = samples[ix];

            if(ix)
                drawLine<number>(color, previous.x, previous.y, current.x, current.y);

            drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true, number>(color_f_t{1.0,0.0,0.0},
                                                                                current.x, current.y, circle_diameter, 255);

            previous = current;
        }

        drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true, number>(color_f_t{0.0,0.0,1.0},
                                                                            point_anchor[0].x, point_anchor[0].y,
                                                                                    circle_diameter, 255);

        drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(color_f_t{0.0,0.0,1.0},
                                                                            point_anchor[1].x, point_anchor[1].y,
                                                                            circle_diameter, 255);

        drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(color_f_t{0.0,0.0,1.0},
                                                                            point_anchor[2].x, point_anchor[2].y,
                                                                            circle_diameter, 255);
        if(type==c::Type::Cubic ) {
            drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(color_f_t{0.0,0.0,1.0},
                                                                                point_anchor[3].x, point_anchor[3].y,
                                                                                circle_diameter, 255);

        }

    }

}

template<typename P, typename CODER>
template <typename BlendMode,
        typename PorterDuff,
        bool antialias>
void Canvas<P, CODER>::drawPolygon(vec2_32i *points,
                                   Canvas::index size,
                                   precision precision,
                                   opacity opacity,
                                   polygons::hints hint
                                   ) {

    TrianglesIndices type;
    // currently static on the stack
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;

    switch (hint) {

        case hints::CONCAVE:
        case hints::SIMPLE:
        {
            type = antialias ? triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY :
                   triangles::TrianglesIndices::TRIANGLES;
//            tessellation::EarClippingTriangulation ear{false};
//            tessellation::ear_clipping_triangulation::compute(points,
//                                                              size,
//                                                              indices,
//                                                              &boundary_buffer,
//                                                              type
//            );

            break;
        }
        case hints::CONVEX:
        {
            type = antialias ? triangles::TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY :
                   triangles::TrianglesIndices::TRIANGLES_FAN;
            tessellation::FanTriangulation::compute(points,
                                                    size,
                                                    indices,
                                                    &boundary_buffer,
                                                    type
            );

            break;
        }
        case hints::NON_SIMPLE:
        case hints::SELF_INTERSECTING:
        default:
            return;
//            throw std::runtime_error("Non-Simple polygons are not supported yet !!!");
            break;
    }

    // draw triangles batch
    drawTriangles<BlendMode, PorterDuff, antialias>(
            RED,
            points,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            opacity,
            precision);

//    return;
    // draw triangulation
    drawTrianglesWireframe(BLACK,
                           points,
                           indices.data(),
                           indices.size(),
                           type,
                           255,
                           precision);

}

template<typename P, typename CODER>
template <typename BlendMode,
        typename PorterDuff,
        bool antialias>
void Canvas<P, CODER>::drawPolygon(vec2_f *points,
                                   Canvas::index size,
                                   opacity opacity,
                                   polygons::hints hint
                                   ) {
    TrianglesIndices type;
    // currently static on the stack
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;

    switch (hint) {

        case hints::CONCAVE:
        case hints::SIMPLE:
        {
            type = antialias ? triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY :
                   triangles::TrianglesIndices::TRIANGLES;
            tessellation::ear_clipping_triangulation<float>::compute(points,
                                                              size,
                                                              indices,
                                                              &boundary_buffer,
                                                              type
            );

            break;
        }
        case hints::CONVEX:
        {
            type = antialias ? triangles::TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY :
                   triangles::TrianglesIndices::TRIANGLES_FAN;
            tessellation::FanTriangulation::compute(points,
                                                    size,
                                                    indices,
                                                    &boundary_buffer,
                                                    type
            );

            break;
        }
        case hints::NON_SIMPLE:
        case hints::SELF_INTERSECTING:
        default:
            return;
//            throw std::runtime_error("Non-Simple polygons are not supported yet !!!");
            break;
    }

    // draw triangles batch
    drawTriangles<BlendMode, PorterDuff, antialias>(
            RED,
            points,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            opacity);

//    return;
    // draw triangulation
    drawTrianglesWireframe(BLACK,
                           points,
                           indices.data(),
                           indices.size(),
                           type,
                           255);
}


