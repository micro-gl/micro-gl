#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#include <microgl/Canvas.h>

template<typename P, typename CODER>
Canvas<P, CODER>::Canvas(Bitmap<P, CODER> *$bmp)
                        : _width{$bmp->width()}, _height{$bmp->height()}, _bitmap_canvas($bmp) {
    uint8_t alpha_bits = CODER::alpha_bits();//coder()->bits_per_alpha();
    _flag_hasNativeAlphaChannel = alpha_bits!=0;
    // fix alpha bits depth in case we don't natively
    // support alpha, this is correct because we want to
    // support compositing even if the surface is opaque.
    _alpha_bits_for_compositing = _flag_hasNativeAlphaChannel ? alpha_bits : 8;
    _max_alpha_value = (1<<_alpha_bits_for_compositing) - 1;
}

template<typename P, typename CODER>
Canvas<P, CODER>::Canvas(int width, int height) :
            Canvas<P, CODER>(new Bitmap<P, CODER>(width, height)) {

}

template<typename P, typename CODER>
inline const coder::PixelCoder<P, CODER> &Canvas<P, CODER>::coder() const {
    return _bitmap_canvas->coder();
}

template<typename P, typename CODER>
inline Bitmap<P, CODER> *Canvas<P, CODER>::bitmapCanvas() const {
    return _bitmap_canvas;
}

template<typename P, typename CODER>
unsigned int Canvas<P, CODER>::sizeofPixel() const {
    return sizeof(P{});
}

template<typename P, typename CODER>
P &Canvas<P, CODER>::getPixel(int x, int y) const {
    // this is not good for high performance loop, cannot be inlined
    return _bitmap_canvas->readAt(y*_width + x);
}

template<typename P, typename CODER>
P &Canvas<P, CODER>::getPixel(int index) const {
    // this is not good for high performance loop, cannot be inlined
    return _bitmap_canvas->readAt(index);
}

template<typename P, typename CODER>
bool Canvas<P, CODER>::hasNativeAlphaChannel() const {
    return _flag_hasNativeAlphaChannel;
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int x, int y, color_t & output)  const {
    this->_bitmap_canvas->decode(x, y, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int index, color_t & output)  const {
    this->_bitmap_canvas->decode(index, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int x, int y, color_f_t & output)  const {
    this->_bitmap_canvas->decode(x, y, output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::getPixelColor(int index, color_f_t & output)  const {
    this->_bitmap_canvas->decode(index, output);
}

template<typename P, typename CODER>
int Canvas<P, CODER>::width() const {
    return _width;
}

template<typename P, typename CODER>
int Canvas<P, CODER>::height() const {
    return _height;
}

template<typename P, typename CODER>
P *Canvas<P, CODER>::pixels()  const {
    return _bitmap_canvas->data();
}

template<typename P, typename CODER>
void Canvas<P, CODER>::clear(const color_f_t &color) {
    P output;
    _bitmap_canvas->coder().encode(color, output);
    _bitmap_canvas->fill(output);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::clear(const color_t &color) {
    P output;
    _bitmap_canvas->coder().encode(color, output);
    _bitmap_canvas->fill(output);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_f_t &val, int x, int y, float opacity) {
    color_t color_int{};
    coder().convert(val, color_int);
    blendColor<BlendMode, PorterDuff>(val, y*_width + x, opacity);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_t &val, int x, int y, opacity_t opacity) {
    blendColor<BlendMode, PorterDuff>(val, y*_width + x, opacity);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_t &val, int index, opacity_t opacity) {
    // we assume that the color conforms to the same pixel-coder. but we are flexible
    // for alpha channel. if coder does not have an alpha channel, the color itself may
    // have non-zero alpha channel, for which we emulate 8-bit alpha processing and also pre
    // multiply result with alpha
    color_t result;
    P output;
    const bool none_compositing = microgl::traits::is_same<PorterDuff, porterduff::None>::value;
    const bool skip_blending =microgl::traits::is_same<BlendMode, blendmode::Normal>::value;
    const bool skip_all= skip_blending && none_compositing && opacity == 255;
    if(!skip_all){
        color_t backdrop, blended;
        const color_t & src = val;

        // get backdrop color if blending or compositing is required
        if(!skip_blending || !none_compositing)
            getPixelColor(index, backdrop);

        // we assume that either they are the same or one of them is zero, this is FASTER then comparison.
        // if we don't own a native alpha channel, check if the color has a suggestion for alpha channel.
        bits alpha_bits = coder().alpha_bits() | val.a_bits;// || _alpha_bits_for_compositing;
        if(alpha_bits) blended.a = src.a;
        else {
            blended.a= 255;
            alpha_bits=8;
        }

        // fix alpha bits depth in case we don't natively
        // support alpha, this is correct because we want to
        // support compositing even if the surface is opaque.
        if(!hasNativeAlphaChannel())
            backdrop.a = _max_alpha_value;

        // if blend-mode is normal or the backdrop is completely transparent
        // then we don't need to blend.
        // the first conditional should be resolved at compile-time therefore it is zero cost !!!
        // this will help with avoiding the inner conditional of the backdrop alpha, the normal
        // blending itself is zero-cost itself, but after it there is a branching which
        // is unpredictable, therefore avoiding at compile-time is great.

        // if we are normal then do nothing
        if(!skip_blending && backdrop.a!=0) { //  or backdrop alpha is zero is also valid

            BlendMode::blend(backdrop, src, blended,
                             coder().red_bits(),
                             coder().green_bits(),
                             coder().blue_bits());

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

        }
        else {
            // skipped blending therefore use src color
            blended.r = src.r;
            blended.g = src.g;
            blended.b = src.b;
        }

        // I fixed opacity is always 8 bits no matter what the alpha depth of the native canvas
        if(opacity < 255)
            blended.a =  (blended.a * opacity)>>8;

        // finally alpha composite with Porter-Duff equations,
        // this should be zero-cost for None option with compiler optimizations
        // if we do not own a native alpha channel, then please keep the composited result
        // with premultiplied alpha, this is why we composite for None option, because it performs
        // alpha multiplication
        PorterDuff::composite(backdrop, blended, result, alpha_bits, !hasNativeAlphaChannel());
    } else
        result = val;

    coder().encode(result, output);

    drawPixel(output, index);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawPixel(const P & val, int x, int y) {
    int index= y*width()+x;
    _bitmap_canvas->writeAt(val, index);
}

template<typename P, typename CODER>
inline void Canvas<P, CODER>::drawPixel(const P & val, int index) {
    _bitmap_canvas->writeAt(val, index);
}

// fast common graphics shapes like circles and rounded rectangles

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S1, typename S2>
void Canvas<P, CODER>::drawCircle(const sampling::sampler<S1> & sampler_fill,
                                  const sampling::sampler<S2> & sampler_stroke,
                                  const number1 &centerX, const number1 &centerY,
                                  const number1 &radius, const number1 &stroke_size, opacity_t opacity,
                                  const number2 &u0, const number2 &v0,
                                  const number2 &u1, const number2 &v1) {
    drawRoundedQuad<BlendMode, PorterDuff, antialias, number1, number2, S1, S2>(sampler_fill, sampler_stroke,
                                                      centerX-radius, centerY-radius,
                                                      centerX+radius+0, centerY+radius+0,
                                                      radius, stroke_size, opacity,
                                                      u0, v0, u1, v1);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S1, typename S2>
void Canvas<P, CODER>::drawRoundedQuad(const sampling::sampler<S1> & sampler_fill,
                                       const sampling::sampler<S2> & sampler_stroke,
                                       const number1 & left, const number1 & top,
                                       const number1 & right, const number1 & bottom,
                                       const number1 & radius, const number1 & stroke_size,
                                       Canvas::opacity_t opacity,
                                       const number2 & u0, const number2 & v0,
                                       const number2 & u1, const number2 & v1) {
    const precision p = 0; const precision p_uv = 16;
#define f_p(x) microgl::math::to_fixed((x), p)
#define f_uv(x) microgl::math::to_fixed((x), p_uv)
    drawRoundedQuad<BlendMode, PorterDuff, antialias>(sampler_fill, sampler_stroke, f_p(left), f_p(top),f_p(right), f_p(bottom),
                                                      f_p(radius), f_p(stroke_size), f_uv(u0), f_uv(v0), f_uv(u1), f_uv(v1),
                                                      p, p_uv, opacity);
#undef f_uv
#undef f_p
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename S1, typename S2>
void Canvas<P, CODER>::drawRoundedQuad(const sampling::sampler<S1> & sampler_fill,
                                       const sampling::sampler<S2> & sampler_stroke,
                                       int left, int top,
                                       int right, int bottom,
                                       int radius, int stroke_size,
                                       l64 u0, l64 v0, l64 u1, l64 v1,
                                       precision sub_pixel_precision, precision uv_p,
                                       Canvas::opacity_t opacity) {
    opacity=255;
    const precision p = sub_pixel_precision;
    const int stroke = stroke_size;//(10<<p)/1;
    const int aa_range = (1<<p)/1;
    const int radius_squared=(l64(radius)*(radius))>>p;
    const int stroke_radius = (l64(radius-stroke)*(radius-stroke))>>p;
    const int outer_aa_radius = (l64(radius+aa_range)*(radius+aa_range))>>p;
    const int outer_aa_bend = outer_aa_radius-radius_squared;
    const int inner_aa_radius = (l64(radius-stroke-aa_range)*(radius-stroke-aa_range))>>p;
    const int inner_aa_bend = stroke_radius-inner_aa_radius;
    const bool apply_opacity = opacity!=255;
    // dimensions in two spaces, one in raster spacee for optimization
    const int left_=left, top_=top, right_=right, bottom_=bottom;
    const int left_r=left_>>p, top_r=top_>>p, right_r=right_>>p, bottom_r=bottom_>>p;
    bool degenerate= left_r==right_r || top_r==bottom_r;
    if(degenerate) return;
    const int step = int(1)<<p;
    const int half = 0;//step>>1;
    const l64 du = (u1-u0)/(right_r-left_r);
    const l64 dv = (v1-v0)/(bottom_r-top_r);
    color_t color;

#define g1(x, y, u, v, s, o) \
            if((x)>=0 && (x)<_width && (y)>=0 && (y)<_height) { \
                color_t color; \
                s.sample(u, v, uv_p, color); \
                blendColor<BlendMode, PorterDuff>((color), (x), (y), (o)); \
            } \

#define g2(x, y, u, v, s, o) { \
            int x_1=(x)>>p, y_1=y>>p, x_2=(right_-(x-left_))>>p, y_2= (bottom_-(y-top_))>>p; \
            g1(x_1, y_1, u,       v,       (s), (o)) \
            g1(x_2, y_1, u0+(right_r-x_1)*du, v,       (s), (o)) \
            g1(x_1, y_2, u,       v0+(bottom_r-y_1)*dv, (s), (o)) \
            g1(x_2, y_2, u0+(right_r-x_1)*du,       v0+(bottom_r-y_1)*dv, (s), (o)) \
            } \

    // this draws the four rounded parts, it could be faster but then it will also be
    // much complex.
    for (int y = top_, v=v0; y < top_+radius; y+=step, v+=dv) {
        for (int x = left_, u=u0; x < left_+radius; x+=step, u+=du) {
            const bool inside_rounded_part = x<=(left_+radius) && y<=(top_+radius);
            if(inside_rounded_part) {
                int dx = x- half - (left_+radius), dy = y- half - (top_+radius);
                const int distance_squared = ((l64(dx) * dx) >> p) + ((l64(dy) * dy) >> p);
                const bool inside_radius = (distance_squared - radius_squared) <= 0;
                if (inside_radius) {
                    g2(x, y, u, v, sampler_fill, opacity)
                    const bool inside_stroke_disk = (distance_squared - stroke_radius) >= 0;
                    if (inside_stroke_disk) // inside stroke disk
                        g2(x, y, u, v, sampler_stroke, opacity)
                    else { // outside stroke disk, let's test for aa disk or radius inclusion
                        const int delta_inner_aa = -inner_aa_radius + distance_squared;
                        const bool inside_inner_aa_ring = delta_inner_aa >= 0;
                        if (antialias && inside_inner_aa_ring) {
                            // scale inner to 8 bit and then convert to integer
                            uint8_t blend = ((delta_inner_aa) << (8)) / inner_aa_bend;
                            if (apply_opacity) blend = (blend * opacity) >> 8;
                            g2(x, y, u, v, sampler_stroke, blend);
                        }
                    }
                } else if (antialias) { // we are outside the main radius
                    const int delta_outer_aa = outer_aa_radius - distance_squared;
                    const bool inside_outer_aa_ring = delta_outer_aa >= 0;
                    if (inside_outer_aa_ring) {
                        // scale inner to 8 bit and then convert to integer
                        uint8_t blend = ((delta_outer_aa) << (8)) / outer_aa_bend;
                        if (apply_opacity) blend = (blend * opacity) >> 8;
                        g2(x, y, u, v, sampler_stroke, blend);
                    }
                }
            }
        }
    }
//return;
#undef g1
#undef g2
#define maX(a, b) functions::max(a, b)
#define miN(a, b) functions::min(a, b)
    { // center
        const int ll=left_+radius, tt=top_+radius, rr=right_-radius, bb=bottom_-radius;
        const int ll_r= maX(0, ll>>p), tt_r= maX(0, tt>>p), rr_r= miN(_width-1, rr>>p), bb_r= miN(_height-1, bb>>p);
        int index = tt_r * _width; const l64 u_start=u0 + du*(ll_r-left_r); l64 v= v0 + dv*(tt_r-top_r);
        for (int y=tt_r; y<=bb_r; y++, v+=dv, index+=_width) {
            for (int x=ll_r, u=u_start; x<=rr_r; x++, u+=du) {
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
            }
        }
    }
    { // top
        const int ll=left_+radius, tt=top_, rr=right_-radius, bb=top_+radius;
        const int ll_r= maX(0, ll>>p), tt_r= maX(0, tt>>p), rr_r= miN(_width-1, rr>>p), bb_r= miN(_height-1, bb>>p);
        int index = tt_r * _width; const l64 u_start=u0 + du*(ll_r-left_r); l64 v= v0 + dv*(tt_r-top_r);
        for (int y=tt_r, yy=tt; y<bb_r; y++, yy+=step, v+=dv, index+=_width) {
            for (int x=ll_r, u=u_start; x<=rr_r; x++, u+=du) {
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                if(yy<=tt+stroke+0) {
                    sampler_stroke.sample(u, v, uv_p, color);
                    blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                }
            }
        }
    }
    { // bottom
        const int ll=left_+radius, tt=bottom_-radius+0, rr=right_-radius, bb=bottom_;
        const int ll_r= maX(0, ll>>p), tt_r= maX(0, tt>>p), rr_r= miN(_width-1, rr>>p), bb_r= miN(_height-1, bb>>p);
        int index = tt_r * _width; const l64 u_start=u0 + du*(ll_r-left_r); l64 v= v0 + dv*(tt_r-top_r);
        for (int y=tt_r, yy=tt; y<=bb_r; y++, yy+=step, v+=dv, index+=_width) {
            for (int x=ll_r, u=u_start; x<=rr_r; x++, u+=du) {
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                if(yy>=bb-stroke) {
                    sampler_stroke.sample(u, v, uv_p, color);
                    blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                }
            }
        }
    }
    { // left
        const int ll=left_, tt=top_+radius, rr=ll+radius, bb=bottom_-radius;
        const int ll_r= maX(0, ll>>p), tt_r= maX(0, tt>>p), rr_r= miN(_width-1, rr>>p), bb_r= miN(_height-1, bb>>p);
        int index = tt_r * _width; const l64 u_start=u0 + du*(ll_r-left_r); l64 v= v0 + dv*(tt_r-top_r);
        for (int y=tt_r, yy=tt; y<=bb_r; y++, yy+=step, v+=dv, index+=_width) {
            for (int x=ll_r, xx=ll, u=u_start; x<rr_r; x++, xx+=step, u+=du) {
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                if(xx<=ll+stroke) {
                    sampler_stroke.sample(u, v, uv_p, color);
                    blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                }
            }
        }
    }
    { // right
        const int ll=right_-radius+step, tt=top_+radius, rr=right_, bb=bottom_-radius;
        const int ll_r= maX(0, ll>>p), tt_r= maX(0, tt>>p), rr_r= miN(_width-1, rr>>p), bb_r= miN(_height-1, bb>>p);
        int index = tt_r * _width; const l64 u_start=u0 + du*(ll_r-left_r); l64 v= v0 + dv*(tt_r-top_r);
        for (int y=tt_r, yy=tt; y<=bb_r; y++, yy+=step, v+=dv, index+=_width) {
            for (int x=ll_r, xx=ll, u=u_start; x<=rr_r; x++, xx+=step, u+=du) {
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                if(xx>=rr-stroke) {
                    sampler_stroke.sample(u, v, uv_p, color);
                    blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                }
            }
        }
    }
#undef maX
#undef miN
}

// Triangles

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTriangles(const color_f_t &color,
                                     const vec2<number> *vertices,
                                     const index *indices,
                                     const boundary_info * boundary_buffer,
                                     const index size,
                                     const enum indices type,
                                     const opacity_t opacity) {
#define f microgl::math::to_fixed
    const precision p = 8;
    triangles::iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
          [&](const index &idx, const index &first_index, const index &second_index, const index &third_index) {
              const bool aa_2d= boundary_buffer!=nullptr;
              bool aa_first_edge=true, aa_second_edge=true, aa_third_edge=true;
              if(aa_2d) {
                  const boundary_info aa_info = boundary_buffer[idx];
                  aa_first_edge = triangles::classify_boundary_info(aa_info, 0);
                  aa_second_edge = triangles::classify_boundary_info(aa_info, 1);
                  aa_third_edge = triangles::classify_boundary_info(aa_info, 2);
              }
              drawTriangle<BlendMode, PorterDuff, antialias>(color,
                      f(vertices[first_index].x, p), f(vertices[first_index].y, p),
                      f(vertices[second_index].x, p), f(vertices[second_index].y, p),
                      f(vertices[third_index].x, p), f(vertices[third_index].y, p),
                      opacity, p, aa_first_edge, aa_second_edge, aa_third_edge);
          });
#undef f
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S>
void Canvas<P, CODER>::drawTriangles(const sampling::sampler<S> &sampler,
                                     const vec2<number1> *vertices,
                                     const vec2<number2> *uvs,
                                     const index *indices,
                                     const boundary_info * boundary_buffer,
                                     const index size,
                                     const enum indices type,
                                     const opacity_t opacity) {
#define f microgl::math::to_fixed
    const precision p = 8;
    const precision uv_p = 16;
    const auto zero_uv=vec2<number2>{0,0};
    triangles::iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
          [&](const index &idx, const index &first_index, const index &second_index, const index &third_index) {
              const bool aa_2d= boundary_buffer!=nullptr;
              bool aa_first_edge=true, aa_second_edge=true, aa_third_edge=true;
              if(aa_2d) {
                  const boundary_info aa_info = boundary_buffer[idx];
                  aa_first_edge = triangles::classify_boundary_info(aa_info, 0);
                  aa_second_edge = triangles::classify_boundary_info(aa_info, 1);
                  aa_third_edge = triangles::classify_boundary_info(aa_info, 2);
              }
              const auto & v1= vertices[first_index], v2=vertices[second_index], v3=vertices[third_index];
              const auto & uv1= uvs?uvs[first_index]:zero_uv, uv2=uvs?uvs[second_index]:zero_uv, uv3=uvs?uvs[third_index]:zero_uv;
              drawTriangle<BlendMode, PorterDuff, antialias, false, S>(sampler,
                      f(v1.x,p), f(v1.y,p), f(uv1.x, uv_p), f(uv1.y, uv_p), 0,
                      f(v2.x,p), f(v2.y,p), f(uv2.x, uv_p), f(uv2.y, uv_p), 0,
                      f(v3.x,p), f(v3.y,p), f(uv3.x, uv_p), f(uv3.y, uv_p), 0,
                      opacity, p, uv_p, aa_first_edge, aa_second_edge, aa_third_edge);
          });
#undef f
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number>
void Canvas<P, CODER>::drawTriangles(shader_base<impl, vertex_attr, varying, number> &shader,
                                     const vertex_attr *vertex_buffer,
                                     const index *indices,
                                     const boundary_info * boundary_buffer,
                                     const index size,
                                     const enum indices type,
                                     const triangles::face_culling & culling,
                                     long long * depth_buffer,
                                     const opacity_t opacity) {
    triangles::iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
          [&](const index &idx, const index &first_index, const index &second_index, const index &third_index) {
              const bool aa_2d= boundary_buffer!=nullptr;
              bool aa_first_edge=true, aa_second_edge=true, aa_third_edge=true;
              if(aa_2d) {
                  const boundary_info aa_info = boundary_buffer[idx];
                  aa_first_edge = triangles::classify_boundary_info(aa_info, 0);
                  aa_second_edge = triangles::classify_boundary_info(aa_info, 1);
                  aa_third_edge = triangles::classify_boundary_info(aa_info, 2);
              }
              drawTriangle<BlendMode, PorterDuff, antialias, perspective_correct, depth_buffer_flag>(
                      shader,
                      vertex_buffer[first_index],
                      vertex_buffer[second_index],
                      vertex_buffer[third_index],
                      opacity, culling, depth_buffer,
                      aa_first_edge, aa_second_edge, aa_third_edge);
          });
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTrianglesWireframe(const color_f_t &color,
                                              const vec2<number> *vertices,
                                              const index *indices,
                                              const index size,
                                              const enum indices type,
                                              const opacity_t opacity) {
    triangles::iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
                      [&](const index &idx, const index &first_index, const index &second_index, const index &third_index) {
                          drawTriangleWireframe(color, vertices[first_index], vertices[second_index], vertices[third_index]);
                      });
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTriangleWireframe(const color_f_t &color,
                                             const vec2<number> &p0,
                                             const vec2<number> &p1,
                                             const vec2<number> &p2) {
    drawLine(color, p0.x, p0.y, p1.x, p1.y);
    drawLine(color, p1.x, p1.y, p2.x, p2.y);
    drawLine(color, p2.x, p2.y, p0.x, p0.y);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, typename S>
void Canvas<P, CODER>::drawTriangle(const sampling::sampler<S> &sampler,
                                    int v0_x, int v0_y, int u0, int v0, int q0,
                                    int v1_x, int v1_y, int u1, int v1, int q1,
                                    int v2_x, int v2_y, int u2, int v2, int q2,
                                    const opacity_t opacity, const precision sub_pixel_precision, const precision uv_precision,
                                    bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    l64 area = functions::orient2d(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, sub_pixel_precision);
    if(area==0) return;
    if(area<0) { // convert CCW to CW triangle
        area=-area;
        functions::swap(v1_x, v2_x);
        functions::swap(v1_y, v2_y);
        functions::swap(u1, u2);
        functions::swap(v1, v2);
        functions::swap(q1, q2);
        functions::swap(aa_first_edge, aa_third_edge);
    }

    l64 max_sub_pixel_precision_value = (l64(1)<<sub_pixel_precision) - 1;
    // bounding box
    l64 minX = (functions::min(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    l64 minY = (functions::min(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    l64 maxX = (functions::max(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    l64 maxY = (functions::max(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;

    // anti-alias pad for distance calculation
    precision bits_distance = 0;
    precision bits_distance_complement = 8;
    // max distance to consider in canvas space
    unsigned int max_distance_canvas_space_anti_alias=0;
    // max distance to consider in scaled space
    unsigned int max_distance_scaled_space_anti_alias=0;
    const precision PREC_DIST = 16;

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
            triangles::classifyTopLeftEdges(false, v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    int bias_w0 = top_left.first  ? 0 : -1;
    int bias_w1 = top_left.second ? 0 : -1;
    int bias_w2 = top_left.third  ? 0 : -1;
    // clipping
    minX = functions::max<l64>(0, minX); minY = functions::max<l64>(0, minY);
    maxX = functions::min<l64>(width()-1, maxX); maxY = functions::min<l64>(height()-1, maxY);
    // Barycentric coordinates at minX/minY corner
    vec2<l64> p = { minX, minY };
    vec2<l64> p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };
    // this can produce a 2P bits number if the points form a a perpendicular triangle
    l64 half= l64(1)<<(sub_pixel_precision-1);
    l64 w0_row = functions::orient2d(v0_x, v0_y, v1_x, v1_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w0;
    l64 w1_row = functions::orient2d(v1_x, v1_y, v2_x, v2_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w1;
    l64 w2_row = functions::orient2d(v2_x, v2_y, v0_x, v0_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w2;
    // sub_pixel_precision;
    const precision BITS_UV_COORDS = uv_precision;
    const precision PP = sub_pixel_precision;
    uint8_t MAX_PREC = 63;
    uint8_t LL = MAX_PREC - (sub_pixel_precision + BITS_UV_COORDS);
    uint64_t ONE = ((uint64_t)1)<<LL;
    uint64_t one_area = (ONE) / area;
    // Triangle setup
    // this needs at least (P+1) bits, since the delta is always <= length
    l64 A01 = (v0_y - v1_y), B01 = (v1_x - v0_x);
    l64 A12 = (v1_y - v2_y), B12 = (v2_x - v1_x);
    l64 A20 = (v2_y - v0_y), B20 = (v0_x - v2_x);
    // AA, 2A/L = h, therefore the division produces a P bit number
    l64 w0_row_h=0, w1_row_h=0, w2_row_h=0;
    l64 A01_h=0, B01_h=0, A12_h=0, B12_h=0, A20_h=0, B20_h=0;

    if(antialias) {
        // lengths of edges, produces a P+1 bits number
        unsigned int length_w0 = microgl::math::distance(v0_x, v0_y, v1_x, v1_y);
        unsigned int length_w1 = microgl::math::distance(v1_x, v1_y, v2_x, v2_y);
        unsigned int length_w2 = microgl::math::distance(v0_x, v0_y, v2_x, v2_y);

        A01_h = ((int64_t)(v0_y - v1_y)<<PREC_DIST)/length_w0, B01_h = ((int64_t)(v1_x - v0_x)<<PREC_DIST)/length_w0;
        A12_h = ((int64_t)(v1_y - v2_y)<<PREC_DIST)/length_w1, B12_h = ((int64_t)(v2_x - v1_x)<<PREC_DIST)/length_w1;
        A20_h = ((int64_t)(v2_y - v0_y)<<PREC_DIST)/length_w2, B20_h = ((int64_t)(v0_x - v2_x)<<PREC_DIST)/length_w2;

        w0_row_h = ((int64_t)(w0_row)<<PREC_DIST)/length_w0;
        w1_row_h = ((int64_t)(w1_row)<<PREC_DIST)/length_w1;
        w2_row_h = ((int64_t)(w2_row)<<PREC_DIST)/length_w2;
    }

    int index = p.y * _width;
    for (p.y = minY; p.y <= maxY; p.y++) {
        l64 w0 = w0_row;
        l64 w1 = w1_row;
        l64 w2 = w2_row;
        l64 w0_h=0,w1_h=0,w2_h=0;
        if(antialias) {
            w0_h = w0_row_h;
            w1_h = w1_row_h;
            w2_h = w2_row_h;
        }
        for (p.x = minX; p.x <= maxX; p.x++) {
            bool should_sample=false;
            uint8_t blend=opacity;
            if((w0|w1|w2)>=0) should_sample=true;
            else if(antialias) { // cheap AA based on SDF
                const l64 distance = functions::min(w0_h, w1_h, w2_h);
                l64 delta = distance+max_distance_scaled_space_anti_alias;
                bool perform_aa = delta>=0 && (aa_all_edges || ((distance == w0_h) && aa_first_edge) ||
                                               ((distance == w1_h) && aa_second_edge) ||
                                               ((distance == w2_h) && aa_third_edge));
                if (perform_aa) {
                    should_sample = true;
                    blend = functions::clamp<int>(((uint64_t)(delta << bits_distance_complement))>>PREC_DIST,0, 255);
                    if (opacity < _max_alpha_value) blend = (blend * opacity) >> 8; // * 257>>16 - blinn method
                }
            }

            if(should_sample) {
                int u_i, v_i;
                auto u_fixed = ((w0*u2)>>PP) + ((w1*u0)>>PP) + ((w2*u1)>>PP);
                auto v_fixed = ((w0*v2)>>PP) + ((w1*v0)>>PP) + ((w2*v1)>>PP);
                if(perspective_correct) {
                    auto q_fixed = ((w0*q2)>>PP) + ((w1*q0)>>PP) + ((w2*q1)>>PP);
                    u_i = (u_fixed<<BITS_UV_COORDS)/q_fixed;
                    v_i = (v_fixed<<BITS_UV_COORDS)/q_fixed;
                } else {
                    // stabler rasterizer, that will not overflow fast
                    //u_i = (u_fixed)/uint64_t(area>>PP);
                    //v_i = (v_fixed)/uint64_t(area>>PP);
                    u_i = (u_fixed*one_area)>>(LL-PP);
                    v_i = (v_fixed*one_area)>>(LL-PP);
                }
                if(antialias) {
                    u_i = functions::clamp<int>(u_i, 0, 1<<BITS_UV_COORDS);
                    v_i = functions::clamp<int>(v_i, 0, 1<<BITS_UV_COORDS);
                }
                color_t col_bmp;
                sampler.sample(u_i, v_i, BITS_UV_COORDS, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
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

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, typename S, typename number1, typename number2>
void Canvas<P, CODER>::drawTriangle(const sampling::sampler<S> & sampler,
                                   const number1 v0_x, const number1 v0_y, number2 u0, number2 v0,
                                   const number1 v1_x, const number1 v1_y, number2 u1, number2 v1,
                                   const number1 v2_x, const number1 v2_y, number2 u2, number2 v2,
                                   const opacity_t opacity,
                                   bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    const precision prec_pixel = 8, prec_uv = 16; const number2 one= number2(1);
#define f_pos(v) microgl::math::to_fixed((v), prec_pixel)
#define f_uv(v) microgl::math::to_fixed((v), prec_uv)
    drawTriangle<BlendMode, PorterDuff, antialias, false, S>(sampler,
            f_pos(v0_x), f_pos(v0_y), f_uv(u0), f_uv(v0), f_uv(one),
            f_pos(v1_x), f_pos(v1_y), f_uv(u1), f_uv(v1), f_uv(one),
            f_pos(v2_x), f_pos(v2_y), f_uv(u2), f_uv(v2), f_uv(one),
            opacity, prec_pixel, prec_uv,
            aa_first_edge, aa_second_edge, aa_third_edge);
#undef f_pos
#undef f_uv
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number>
void Canvas<P, CODER>::drawTriangle(shader_base<impl, vertex_attr, varying, number> &shader,
                                    vertex_attr v0, vertex_attr v1, vertex_attr v2,
                                    const opacity_t opacity, const triangles::face_culling & culling,
                                     long long * depth_buffer,
                                    bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
#define f microgl::math::to_fixed
    // this and drawTriangle_shader_homo_internal is the programmable 3d pipeline
    // compute varying and positions per vertex for interpolation
    varying varying_v0, varying_v1, varying_v2;
    varying varying_v0_clip, varying_v1_clip, varying_v2_clip;
    auto v0_homo_space = shader.vertex(v0, varying_v0);
    auto v1_homo_space = shader.vertex(v1, varying_v1);
    auto v2_homo_space = shader.vertex(v2, varying_v2);
    // compute clipping in homogeneous 4D space
    using clipper= microgl::clipping::homo_triangle_clipper<number>;
    typename clipper::vertices_list result_clipping;
    const bool outside= !clipper::compute(v0_homo_space, v1_homo_space, v2_homo_space, result_clipping);
    if(outside) return;
    const auto triangles= result_clipping.size()-2;
    for (unsigned ix=0; ix < triangles; ++ix) {
        const auto & p0= result_clipping[0].point;
        const auto & p1= result_clipping[ix+1].point;
        const auto & p2= result_clipping[ix+2].point;
        const auto & bary_0= result_clipping[0].bary;
        const auto & bary_1= result_clipping[ix+1].bary;
        const auto & bary_2= result_clipping[ix+2].bary;
        // convert bary to 16 bits fixed points
        constexpr precision p = 15;
        const vec4<long long> bary_0_fixed= {f(bary_0.x, p), f(bary_0.y, p), f(bary_0.z, p), f(bary_0.w, p)};
        const vec4<long long> bary_1_fixed= {f(bary_1.x, p), f(bary_1.y, p), f(bary_1.z, p), f(bary_1.w, p)};
        const vec4<long long> bary_2_fixed= {f(bary_2.x, p), f(bary_2.y, p), f(bary_2.z, p), f(bary_2.w, p)};
        varying_v0_clip.interpolate(varying_v0, varying_v1, varying_v2, bary_0_fixed);
        varying_v1_clip.interpolate(varying_v0, varying_v1, varying_v2, bary_1_fixed);
        varying_v2_clip.interpolate(varying_v0, varying_v1, varying_v2, bary_2_fixed);

        drawTriangle_shader_homo_internal<BlendMode, PorterDuff, antialias, perspective_correct, depth_buffer_flag, impl, vertex_attr, varying, number>(
                shader,
                p0, p1, p2,
                varying_v0_clip, varying_v1_clip, varying_v2_clip,
                opacity, culling, depth_buffer, aa_first_edge, aa_second_edge, aa_third_edge);
    }
#undef f
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number>
void Canvas<P, CODER>::drawTriangle_shader_homo_internal(shader_base<impl, vertex_attr, varying, number> &shader,
                                                         const vec4<number> &p0,  const vec4<number> &p1,  const vec4<number> &p2,
                                                         varying &varying_v0, varying &varying_v1, varying &varying_v2,
                                                         opacity_t opacity, const triangles::face_culling & culling,
                                                         long long * depth_buffer,
                                                         bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    /*
     * given triangle coords in a homogeneous coords, a shader, and corresponding interpolated varying
     * vertex attributes. we pass varying because somewhere in the pipeline we might have clipped things
     * in homogeneous space and therefore had to update/correct the vertex attributes.
     */
    const precision sub_pixel_precision = 4;
#define f microgl::math::to_fixed
    varying interpolated_varying;
    // perspective divide by w -> NDC space
    // todo: bail out if w==0
    const auto v0_ndc = p0/p0.w;
    const auto v1_ndc = p1/p1.w;
    const auto v2_ndc = p2/p2.w;
    // viewport transform: NDC space -> raster space
    const number w= width();
    const number h= height();
    number one = number(1), two=number(2);
    vec3<number> v0_viewport = {((v0_ndc.x + one)*w)/two, h - ((v0_ndc.y + one)*h)/two, (v0_ndc.z + one)/two};
    vec3<number> v1_viewport = {((v1_ndc.x + one)*w)/two, h - ((v1_ndc.y + one)*h)/two, (v1_ndc.z + one)/two};
    vec3<number> v2_viewport = {((v2_ndc.x + one)*w)/two, h - ((v2_ndc.y + one)*h)/two, (v2_ndc.z + one)/two};

    // collect values for interpolation as fixed point integers
    int v0_x= f(v0_viewport.x, sub_pixel_precision), v0_y= f(v0_viewport.y, sub_pixel_precision);
    int v1_x= f(v1_viewport.x, sub_pixel_precision), v1_y= f(v1_viewport.y, sub_pixel_precision);
    int v2_x= f(v2_viewport.x, sub_pixel_precision), v2_y= f(v2_viewport.y, sub_pixel_precision);
    const int w_bits= 18; const l64 one_w= (l64(1) << (w_bits << 1)); // negate z because camera is looking negative z axis
    l64 v0_w= one_w / f(p0.w, w_bits), v1_w= one_w / f(p1.w, w_bits), v2_w= one_w / f(p2.w, w_bits);
    const int z_bits= 24; const l64 one_z= (l64(1) << (z_bits)); // negate z because camera is looking negative z axis
    l64 v0_z= f(v0_viewport.z, z_bits), v1_z= f(v1_viewport.z, z_bits), v2_z= f(v2_viewport.z, z_bits);

    l64 area = functions::orient2d(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, sub_pixel_precision);
    // infer back-face culling
    const bool ccw = area<0;
    if(area==0) return; // discard degenerate triangles
    if(ccw && culling==triangles::face_culling::ccw) return;
    if(!ccw && culling==triangles::face_culling::cw) return;
    if(ccw) { // convert CCW to CW triangle
        functions::swap(v1_x, v2_x);
        functions::swap(v1_y, v2_y);
        area = -area;
    } else { // flip vertically
        functions::swap(varying_v1, varying_v2);
        functions::swap(v1_w, v2_w);
        functions::swap(v1_z, v2_z);
    }
    // rotate to match edges
    functions::swap(varying_v0, varying_v1);
    functions::swap(v0_w, v1_w);
    functions::swap(v0_z, v1_z);

#undef f

    // bounding box in raster space
    int max_sub_pixel_precision_value = (1u<<sub_pixel_precision) - 1;
    int minX = (functions::min(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value)>>sub_pixel_precision;
    int minY = (functions::min(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value)>>sub_pixel_precision;
    int maxX = (functions::max(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value)>>sub_pixel_precision;
    int maxY = (functions::max(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value)>>sub_pixel_precision;
    minX = functions::max(0, minX); minY = functions::max(0, minY);
    maxX = functions::min(width()-1, maxX); maxY = functions::min(height()-1, maxY);
    bool outside= maxX<0 || maxY<0 || minX>(width()-1) || minY>(height()-1);
    if(outside) return; // cull in 2d raster window
    // anti-alias SDF configurations
    bits bits_distance = 0;
    bits bits_distance_complement = 8;
    // max distance to consider in canvas space
    unsigned int max_distance_canvas_space_anti_alias=0;
    // max distance to consider in scaled space
    unsigned int max_distance_scaled_space_anti_alias=0;
    bits PREC_DIST = 15;
    bool aa_all_edges = false;
    if(antialias) {
        aa_all_edges = aa_first_edge && aa_second_edge && aa_third_edge;
        bits_distance = 0;
        bits_distance_complement = 8 - bits_distance;
        max_distance_canvas_space_anti_alias = 1u << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias<<PREC_DIST;
    }

    // fill rules configurations
    triangles::top_left_t top_left =
            triangles::classifyTopLeftEdges(false,
                                            v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    int bias_w0 = top_left.first  ? 0 : -1;
    int bias_w1 = top_left.second ? 0 : -1;
    int bias_w2 = top_left.third  ? 0 : -1;
    // Barycentric coordinates at minX/minY corner
    vec2<int> p = { minX, minY };
    vec2<int> p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };
    // Triangle setup, this needs at least (P+1) bits, since the delta is always <= length
    int64_t A01 = (v0_y - v1_y), B01 = (v1_x - v0_x);
    int64_t A12 = (v1_y - v2_y), B12 = (v2_x - v1_x);
    int64_t A20 = (v2_y - v0_y), B20 = (v0_x - v2_x);

    int w0_row = functions::orient2d(v0_x, v0_y, v1_x, v1_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w0;
    int w1_row = functions::orient2d(v1_x, v1_y, v2_x, v2_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w1;
    int w2_row = functions::orient2d(v2_x, v2_y, v0_x, v0_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w2;
    // AA, 2A/L = h, therefore the division produces a P bit number
    int64_t w0_row_h=0, w1_row_h=0, w2_row_h=0;
    int64_t A01_h=0, B01_h=0, A12_h=0, B12_h=0, A20_h=0, B20_h=0;

    if(antialias) {
        // lengths of edges, produces a P+1 bits number
        unsigned int length_w0 = microgl::math::distance(v0_x, v0_y, v1_x, v1_y);
        unsigned int length_w1 = microgl::math::distance(v1_x, v1_y, v2_x, v2_y);
        unsigned int length_w2 = microgl::math::distance(v0_x, v0_y, v2_x, v2_y);

        A01_h = ((int64_t)(v0_y - v1_y)<<PREC_DIST)/length_w0, B01_h = ((int64_t)(v1_x - v0_x)<<PREC_DIST)/length_w0;
        A12_h = ((int64_t)(v1_y - v2_y)<<PREC_DIST)/length_w1, B12_h = ((int64_t)(v2_x - v1_x)<<PREC_DIST)/length_w1;
        A20_h = ((int64_t)(v2_y - v0_y)<<PREC_DIST)/length_w2, B20_h = ((int64_t)(v0_x - v2_x)<<PREC_DIST)/length_w2;

        w0_row_h = ((int64_t)(w0_row)<<PREC_DIST)/length_w0;
        w1_row_h = ((int64_t)(w1_row)<<PREC_DIST)/length_w1;
        w2_row_h = ((int64_t)(w2_row)<<PREC_DIST)/length_w2;
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
        for (p.x = minX; p.x<=maxX; p.x++) {
            const bool in_closure= (w0|w1|w2)>=0;
            bool should_sample= in_closure;
            auto opacity_sample = opacity;
            auto bary = vec4<l64>{w0, w1, w2, area};
            if(in_closure && perspective_correct) { // compute perspective-correct and transform to sub-pixel-space
                bary.x= (l64(w0)*v0_w)>>w_bits, bary.y= (l64(w1)*v1_w)>>w_bits, bary.z= (l64(w2)*v2_w)>>w_bits;
                bary.w=bary.x+bary.y+bary.z;
                if(bary.w==0) bary.w=1;
            }
            if(antialias && !in_closure) {
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances
                int64_t distance = functions::min(w0_h, w1_h, w2_h);
                int64_t delta = (distance) + max_distance_scaled_space_anti_alias;
                bool perform_aa = aa_all_edges;
                // test edges
                if(!perform_aa) {
                    if(distance==w0_h && aa_first_edge) perform_aa = true;
                    else if(distance==w1_h && aa_second_edge) perform_aa = true;
                    else perform_aa = distance == w2_h && aa_third_edge;
                }
                should_sample= perform_aa && delta>=0;
                if(should_sample) {
                    opacity_t blend = functions::clamp<int64_t>((((int64_t(delta) << bits_distance_complement))>>PREC_DIST),
                                                                0, 255);
                    if (opacity < _max_alpha_value)
                        blend = (blend * opacity) >> 8;
                    opacity_sample= blend;

                    if(perspective_correct) { // compute perspective-correct and transform to sub-pixel-space
                        bary.x= (l64(w0)*v0_w)>>w_bits, bary.y= (l64(w1)*v1_w)>>w_bits, bary.z= (l64(w2)*v2_w)>>w_bits;
                        bary.w=bary.x+bary.y+bary.z;
                        if(bary.w==0) bary.w=1;
                    }
                    // rewrite barycentric coords for AA so it sticks to the edges, seems to work
                    bary.x= functions::clamp<long long>(bary.x, 0, bary.w);
                    bary.y= functions::clamp<long long>(bary.y, 0, bary.w);
                    bary.z= functions::clamp<long long>(bary.z, 0, bary.w);
                    bary.w= bary.x+bary.y+bary.z;
                }
            }
            if(depth_buffer_flag && should_sample) {
//                l64 z= (((v0_z)*bary.x) +((v1_z)*bary.y) +((v2_z)*bary.z))/(bary.w);
//                l64 z= ((v0_z*w0) +(v1_z*w1) +(v2_z*w2))/area;
                l64 z= (long long)(number((v0_z*w0) +(v1_z*w1) +(v2_z*w2))/(area));
//                z_tag= functions::clamp<l64>(z_tag, 0, l64(1)<<44);
                if(z<0 || z>depth_buffer[index + p.x]) should_sample=false;
                else depth_buffer[index + p.x]=z;
            }
            if(should_sample) {
                // cast to user's number types vec4<number> casted_bary= bary;, I decided to stick with l64
                // because other wise this would have wasted bits for Q types although it would have been more elegant.
                interpolated_varying.interpolate(
                        varying_v0,
                        varying_v1,
                        varying_v2, bary);
                auto color = shader.fragment(interpolated_varying);
                blendColor<BlendMode, PorterDuff>(color, index + p.x, opacity_sample);
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

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, typename number1, typename number2, typename S>
void Canvas<P, CODER>::drawQuadrilateral(const sampling::sampler<S> & sampler,
                                         number1 v0_x, number1 v0_y, number2 u0, number2 v0,
                                         number1 v1_x, number1 v1_y, number2 u1, number2 v1,
                                         number1 v2_x, number1 v2_y, number2 u2, number2 v2,
                                         number1 v3_x, number1 v3_y, number2 u3, number2 v3,
                                        const uint8_t opacity) {
    const precision uv_p = 16, pixel_p = 8;
#define f microgl::math::to_fixed
    number2 q0 = 1, q1 = 1, q2 = 1, q3 = 1;
    number1 p0x = v0_x; number1 p0y = v0_y;
    number1 p1x = v1_x; number1 p1y = v1_y;
    number1 p2x = v2_x; number1 p2y = v2_y;
    number1 p3x = v3_x; number1 p3y = v3_y;
    number1 ax = p2x - p0x;
    number1 ay = p2y - p0y;
    number1 bx = p3x - p1x;
    number1 by = p3y - p1y;
    number1 t, s;
    number1 cross = ax * by - ay * bx;
    if (cross != number1(0)) {
        number1 cy = p0y - p1y;
        number1 cx = p0x - p1x;
        s = (ax * cy - ay * cx) / cross;
        if (s > number1(0) && s < number1(1)) {
            t = (bx * cy - by * cx) / cross;
            if (t > number1(0) && t < number1(1)) { // here casting t, s to number2
                q0 = number2(1) / (number2(1) - number2(t));
                q1 = number2(1) / (number2(1) - number2(s));
                q2 = number2(1) / number2(t);
                q3 = number2(1) / number2(s);
            }
        }
    }
    number2 u0_q0 = u0*q0, v0_q0 = v0*q0;
    number2 u1_q1 = u1*q1, v1_q1 = v1*q1;
    number2 u2_q2 = u2*q2, v2_q2 = v2*q2;
    number2 u3_q3 = u3*q3, v3_q3 = v3*q3;
    // perspective correct version
    drawTriangle<BlendMode, PorterDuff, antialias, true, S>(sampler,
          f(v0_x, pixel_p), f(v0_y, pixel_p), f(u0_q0, uv_p), f(v0_q0, uv_p), f(q0, uv_p),
          f(v1_x, pixel_p), f(v1_y, pixel_p), f(u1_q1, uv_p), f(v1_q1, uv_p), f(q1, uv_p),
          f(v2_x, pixel_p), f(v2_y, pixel_p), f(u2_q2, uv_p), f(v2_q2, uv_p), f(q2, uv_p),
          opacity, pixel_p, uv_p, true, true, false);
    drawTriangle<BlendMode, PorterDuff, antialias, true, S>(sampler,
          f(v2_x, pixel_p), f(v2_y, pixel_p), f(u2_q2, uv_p), f(v2_q2, uv_p), f(q2, uv_p),
          f(v3_x, pixel_p), f(v3_y, pixel_p), f(u3_q3, uv_p), f(v3_q3, uv_p), f(q3, uv_p),
          f(v0_x, pixel_p), f(v0_y, pixel_p), f(u0_q0, uv_p), f(v0_q0, uv_p), f(q0, uv_p),
          opacity, pixel_p, uv_p, true, true, false);
#undef f
}

// quads

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, typename number>
void Canvas<P, CODER>::drawQuad(const color_t & color,
                                const number left, const number top,
                                const number right, const number bottom,
                                const opacity_t opacity) {
#define f microgl::math::to_fixed
    const precision p = 4;
    const unsigned int max_sub_pixel_precision_value = (1<<p) - 1;
    int left_   = functions::max<l64>((f(left, p) + max_sub_pixel_precision_value) >> p,   0);
    int top_    = functions::max<l64>((f(top, p) + max_sub_pixel_precision_value) >> p,    0);
    int right_  = functions::min<l64>((f(right, p) + max_sub_pixel_precision_value) >> p,  width()-1);
    int bottom_ = functions::min<l64>((f(bottom, p) + max_sub_pixel_precision_value) >> p, height()-1);
    int index = top_*_width;
    for (int y = top_; y < bottom_; y++, index+=_width) {
        for (int x = left_; x < right_; x++)
            blendColor<BlendMode, PorterDuff>(color, index + x, opacity);
    }
#undef f
}

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff, typename number1, typename number2, typename S>
void Canvas<P, CODER>::drawQuad(const sampling::sampler<S> & sampler,
                                const number1 left, const number1 top,
                                const number1 right, const number1 bottom,
                                const opacity_t opacity,
                                const number2 u0, const number2 v0,
                                const number2 u1, const number2 v1) {
    const precision p_sub = 4, p_uv = 20;
    drawQuad<BlendMode, PorterDuff, S>(sampler,
            microgl::math::to_fixed(left, p_sub), microgl::math::to_fixed(top, p_sub),
            microgl::math::to_fixed(right, p_sub), microgl::math::to_fixed(bottom, p_sub),
            microgl::math::to_fixed(u0, p_uv), microgl::math::to_fixed(v0, p_uv),
            microgl::math::to_fixed(u1, p_uv), microgl::math::to_fixed(v1, p_uv),
            p_sub, p_uv, opacity);
}

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff, typename S>
void Canvas<P, CODER>::drawQuad(const sampling::sampler<S> & sampler,
                                const int left, const int top,
                                const int right, const int bottom,
                                int u0, int v0,
                                int u1, int v1,
                                const precision sub_pixel_precision,
                                const precision uv_precision,
                                const opacity_t opacity) {
    color_t col_bmp{};
    int max = (1<<sub_pixel_precision) - 1;
    int left_   = functions::max(left, (int)0);
    int top_    = functions::max(top, ( int)0);
    int right_  = functions::min(right, (width()-1)<<sub_pixel_precision);
    int bottom_ = functions::min(bottom, (height()-1)<<sub_pixel_precision);
    bool degenerate= left_==right_ || top_==bottom_;
    if(degenerate) return;
    // intersections
    const int u0_ = u0+int((l64(u1-u0)*(left_-left))/(right-left));
    const int v0_ = v0+int((l64(v1-v0)*(top_-top))/(bottom-top));
    const int u1_ = u0+int((l64(u1-u0)*(right_-left))/(right-left));
    const int v1_ = v0+int((l64(v1-v0)*(bottom_-top))/(bottom-top));
    // round and convert to raster space
    left_   = (max + left_  )>>sub_pixel_precision;
    top_    = (max + top_   )>>sub_pixel_precision;
    right_  = (max + right_ )>>sub_pixel_precision;
    bottom_ = (max + bottom_)>>sub_pixel_precision;
    degenerate= left_==right_ || top_==bottom_;
    if(degenerate) return;
    const int du = (u1_-u0_)/(right_ - left_);
    const int dv = (v1_-v0_)/(bottom_ - top_);
    int index=top_*_width;
    for (int y=top_, v=v0_; y<=bottom_; y++, v+=dv, index+=_width) {
        for (int x=left_, u=u0_; x<=right_; x++, u+=du) {
            sampler.sample(u, v, uv_precision, col_bmp);
            blendColor<BlendMode, PorterDuff>(col_bmp, index + x, opacity);
        }
    }
}

template<typename P, typename CODER>
template <typename number1, typename number2, typename S>
void Canvas<P, CODER>::drawMask(const masks::chrome_mode &mode,
                                const sampling::sampler<S> & sampler,
                                const number1 left, const number1 top,
                                const number1 right, const number1 bottom,
                                const number2 u0, const number2 v0,
                                const number2 u1, const number2 v1,
                                const opacity_t opacity) {
    precision p_sub = 4, p_uv = 20;
    drawMask<S>(mode, sampler,
            microgl::math::to_fixed(left, p_sub), microgl::math::to_fixed(top, p_sub),
            microgl::math::to_fixed(right, p_sub), microgl::math::to_fixed(bottom, p_sub),
            microgl::math::to_fixed(u0, p_uv), microgl::math::to_fixed(v0, p_uv),
            microgl::math::to_fixed(u1, p_uv), microgl::math::to_fixed(v1, p_uv),
            p_sub, p_uv, opacity
    );
}

template<typename P, typename CODER>
template <typename S>
void Canvas<P, CODER>::drawMask(const masks::chrome_mode &mode,
                                const sampling::sampler<S> & sampler,
                                const int left, const int top,
                                const int right, const int bottom,
                                int u0, int v0,
                                int u1, int v1,
                                const precision sub_pixel_precision,
                                const precision uv_precision,
                                const opacity_t opacity) {
    color_t col_bmp{};
    int max = (1u<<sub_pixel_precision) - 1;
    int left_   = functions::max(left, (int)0);
    int top_    = functions::max(top, ( int)0);
    int right_  = functions::min(right, (width()-1)<<sub_pixel_precision);
    int bottom_ = functions::min(bottom, (height()-1)<<sub_pixel_precision);
    // intersections
    const int u0_ = u0+int((l64(u1-u0) *(left_-left))/(right-left));
    const int v0_ = v0+int((l64(v1-v0) *(top_-top))/(bottom-top));
    const int u1_ = u0+int((l64(u1-u0) *(right_-left))/(right-left));
    const int v1_ = v0+int((l64(v1-v0) *(bottom_-top))/(bottom-top));
    // round and convert to raster space
    left_   = (max + left_  )>>sub_pixel_precision;
    top_    = (max + top_   )>>sub_pixel_precision;
    right_  = (max + right_ )>>sub_pixel_precision;
    bottom_ = (max + bottom_)>>sub_pixel_precision;
    // increase precision to (uv_precision*2)
    const int du = (u1_-u0_) / (right_ - left_);
    const int dv = (v1_-v0_) / (bottom_ - top_);
    int index = top_ * _width;
    const bits alpha_bits = this->coder().alpha_bits() | 8;
    const channel max_alpha_value = (1<<alpha_bits) - 1;
    for (int y=top_, v=v0_; y<=bottom_; y++, v+=dv, index+=_width) {
        for (int x=left_, u=u0_; x<=right_; x++, u+=du) {
            sampler.sample(u, v, uv_precision, col_bmp);
            channel a=0;
            switch (mode) {
                case masks::chrome_mode::red_channel:
                    a = coder::convert_channel_correct(col_bmp.r, sampler.red_bits(), alpha_bits);
                    break;
                case masks::chrome_mode::red_channel_inverted:
                    a = max_alpha_value - coder::convert_channel_correct(col_bmp.r, sampler.red_bits(), alpha_bits);
                    break;
                case masks::chrome_mode::alpha_channel:
                    a = coder::convert_channel_correct(col_bmp.a, sampler.alpha_bits(), alpha_bits);
                    break;
                case masks::chrome_mode::alpha_channel_inverted:
                    a = max_alpha_value - coder::convert_channel_correct(col_bmp.a, sampler.alpha_bits(), alpha_bits);
                    break;
                case masks::chrome_mode::green_channel:
                    a = coder::convert_channel_correct(col_bmp.g, sampler.green_bits(), alpha_bits);
                    break;
                case masks::chrome_mode::green_channel_inverted:
                    a = max_alpha_value - coder::convert_channel_correct(col_bmp.g, sampler.green_bits(), alpha_bits);
                    break;
                case masks::chrome_mode::blue_channel:
                    a = coder::convert_channel_correct(col_bmp.b, sampler.blue_bits(), alpha_bits);
                    break;
                case masks::chrome_mode::blue_channel_inverted:
                    a = max_alpha_value - coder::convert_channel_correct(col_bmp.b, sampler.blue_bits(), alpha_bits);
                    break;
            }
            col_bmp.r=0, col_bmp.g=0, col_bmp.b=0, col_bmp.a=a,
            col_bmp.r_bits=this->coder().red_bits(), col_bmp.g_bits=this->coder().green_bits(),
            col_bmp.b_bits=this->coder().blue_bits(), col_bmp.a_bits=alpha_bits;
            // re-encode for a different canvas
            blendColor<blendmode::Normal, porterduff::DestinationIn>(col_bmp, index + x, opacity);
        }
    }
}

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S>
void Canvas<P, CODER>::drawPolygon(const sampling::sampler<S> &sampler,
                                   const vec2<number1> *points,
                                   index size, opacity_t opacity,
                                   polygons::hints hint,
                                   const number2 u0, const number2 v0,
                                   const number2 u1, const number2 v1, const bool debug) {
    indices type;
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;
    switch (hint) {
        case hints::CONCAVE:
        case hints::SIMPLE:
        {
            type = antialias ? triangles::indices::TRIANGLES_WITH_BOUNDARY :
                   triangles::indices::TRIANGLES;
            microgl::tessellation::ear_clipping_triangulation<number1>::compute(points,
                                                                                size,
                                                                                indices,
                                                                                &boundary_buffer,
                                                                                type);
            break;
        }
        case hints::CONVEX:
        {
            type = antialias ? triangles::indices::TRIANGLES_FAN_WITH_BOUNDARY :
                   triangles::indices::TRIANGLES_FAN;
            microgl::tessellation::fan_triangulation<number1>::compute(points,
                                                                       size,
                                                                       indices,
                                                                       &boundary_buffer,
                                                                       type);
            break;
        }
        case hints::NON_SIMPLE:
        case hints::SELF_INTERSECTING:
        default:
            return;
    }

    const auto * uvs= uv_map<number1, number2>::compute(points, size, u0, v0, u1, v1);
    // draw triangles batch
    drawTriangles<BlendMode, PorterDuff, antialias, number1, number2, S>(
            sampler,
            points,
            uvs,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            opacity);

//    drawTriangles<BlendMode, PorterDuff, antialias>(
//            color::colors::RED,
//            points,
//            indices.data(),
//            boundary_buffer.data(),
//            indices.size(),
//            type,
//            opacity);

    if(debug)
        drawTrianglesWireframe(color::colors::BLACK,
                               points,
                               indices.data(),
                               indices.size(),
                               type,
                               255);

    if(uvs)
        delete [] uvs;
}

template<typename P, typename CODER>
template<typename number>
void Canvas<P, CODER>::drawLine(const color_f_t &color,
                                number x0, number y0,
                                number x1, number y1) {
    using clipper = microgl::clipping::cohen_sutherland_clipper<number>;
    auto clip =  clipper::compute(x0, y0, x1, y1, number(0), number(0), width(), height());
    if(!clip.inside) return;
    precision p = 8;
    int x0_ = microgl::math::to_fixed(clip.x0, p);
    int y0_ = microgl::math::to_fixed(clip.y0, p);
    int x1_ = microgl::math::to_fixed(clip.x1, p);
    int y1_ = microgl::math::to_fixed(clip.y1, p);
    drawLine(color, x0_, y0_, x1_, y1_, p);
}

template<typename P, typename CODER>
void Canvas<P, CODER>::drawLine(const color_f_t &color,
                                int x0, int y0,
                                int x1, int y1,
                                precision bits) {
    int X0 = x0, Y0 = y0, X1 = x1, Y1=y1;
    color_t color_input{};

    coder().convert(color, color_input);

    unsigned int IntensityBits = 8;
    unsigned int NumLevels = 1u << IntensityBits;
    unsigned int maxIntensity = NumLevels - 1;
    uint32_t IntensityShift, ErrorAdj, ErrorAcc;
    unsigned int ErrorAccTemp, Weighting, WeightingComplementMask;
    int DeltaX, DeltaY, Temp, XDir;//, YDir;
    int one = 1<<bits;
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
        XDir = 1<<bits;
    } else {
        XDir = -(1<<bits);
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

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S>
void Canvas<P, CODER>::drawBezierPatch(const sampling::sampler<S> & sampler,
                                       const vec3<number1> *mesh,
                                       const unsigned uOrder, const unsigned vOrder,
                                       const unsigned uSamples, const unsigned vSamples,
                                       const number2 u0, const number2 v0,
                                       const number2 u1, const number2 v1,
                                       const opacity_t opacity) {
    using tess= microgl::tessellation::bezier_patch_tesselator<number1, number2>;
    dynamic_array<number1> vertices_attributes;
    dynamic_array<index> indices;
    microgl::triangles::indices indices_type;
    tess::compute(mesh, uOrder, vOrder, uSamples, vSamples, vertices_attributes, indices, indices_type,
                    u0, v0, u1, v1);
    const index size = indices.size();
    const index window_size=5;
    const index I_X=0, I_Y=1, I_Z=2, I_U=3, I_V=4;
    if(size==0) return;
#define IND(a) indices[(a)]
    bool even = true;
    for (index ix = 0; ix < size-2; ++ix) {
        // we alternate order inorder to preserve CCW or CW,
        index first_index   = (even ? IND(ix + 0) : IND(ix + 2))*window_size;
        index second_index  = (even ? IND(ix + 1) : IND(ix + 1))*window_size;
        index third_index   = (even ? IND(ix + 2) : IND(ix + 0))*window_size;
        drawTriangle<BlendMode, PorterDuff, antialias>(sampler,
                                                        vertices_attributes[first_index+I_X],
                                                        vertices_attributes[first_index+I_Y],
                                                        vertices_attributes[first_index+I_U],
                                                        vertices_attributes[first_index+I_V],
                                                        vertices_attributes[second_index+I_X],
                                                        vertices_attributes[second_index+I_Y],
                                                        vertices_attributes[second_index+I_U],
                                                        vertices_attributes[second_index+I_V],
                                                        vertices_attributes[third_index+I_X],
                                                        vertices_attributes[third_index+I_Y],
                                                        vertices_attributes[third_index+I_U],
                                                        vertices_attributes[third_index+I_V],
                                                        opacity);
        even = !even;
    }
#undef IND
}

// todo: drawLinePath will be removed once the path maker is ready
template<typename P, typename CODER>
template <typename number>
void
Canvas<P, CODER>::drawLinePath(color_f_t &color,
                               vec2<number> *points,
                               unsigned int size,
                               bool closed_path) {
    index jx = 0;
    for (jx = 0; jx < size; jx++)
        if(jx)
            drawLine(color,
                     points[jx-1].x, points[jx-1].y,
                     points[jx].x, points[jx].y);
    if(closed_path)
        drawLine(color, points[0].x, points[0].y, points[jx - 1].x, points[jx- 1].y);
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
            if(ix) drawLine<number>(color, previous.x, previous.y, current.x, current.y);
            drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true, number>(color_f_t{1.0,0.0,0.0,1.0},
                                                                                current.x, current.y, circle_diameter, 255);
            previous = current;
        }
        drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true, number>(color_f_t{0.0,0.0,1.0,1.0},
                                                                            point_anchor[0].x, point_anchor[0].y,
                                                                                    circle_diameter, 255);
        drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(color_f_t{0.0,0.0,1.0,1.0},
                                                                            point_anchor[1].x, point_anchor[1].y,
                                                                            circle_diameter, 255);
        drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(color_f_t{0.0,0.0,1.0,1.0},
                                                                            point_anchor[2].x, point_anchor[2].y,
                                                                            circle_diameter, 255);
        if(type==c::Type::Cubic ) {
            drawCircle<blendmode::Normal, porterduff::SourceOverOnOpaque, true>(color_f_t{0.0,0.0,1.0,1.0},
                                                                                point_anchor[3].x, point_anchor[3].y,
                                                                                circle_diameter, 255);
        }
    }
}
