#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

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
inline const coder::PixelCoder<P, CODER> &Canvas<P, CODER>::coder() {
    return _bitmap_canvas->coder();
}

template<typename P, typename CODER>
inline Bitmap<P, CODER> *Canvas<P, CODER>::bitmapCanvas() {
    return _bitmap_canvas;
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
bool Canvas<P, CODER>::hasNativeAlphaChannel() const {
    return _flag_hasNativeAlphaChannel;
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
int Canvas<P, CODER>::width() const {
    return _width;
}

template<typename P, typename CODER>
int Canvas<P, CODER>::height() const {
    return _height;
}

template<typename P, typename CODER>
P *Canvas<P, CODER>::pixels() {
    return _bitmap_canvas->data();
}

template<typename P, typename CODER>
void Canvas<P, CODER>::clear(const color_f_t &color) {
    P output;
    _bitmap_canvas->coder().encode(color, output);
    _bitmap_canvas->fill(output);
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff>
inline void Canvas<P, CODER>::blendColor(const color_f_t &val, int x, int y, float opacity) {
    color_t color_int{};
    coder().convert(val, color_int);
    blendColor<BlendMode, PorterDuff>(coder().val, y*_width + x, opacity);
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
    int index = (y * width() + x);
    _bitmap_canvas->writeAt(val, index);
}

template<typename P, typename CODER>
inline void Canvas<P, CODER>::drawPixel(const P & val, int index) {
    _bitmap_canvas->writeAt(val, index);
}

// fast common graphics shapes like circles and rounded rectangles

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawCircle(const color_f_t & color,
                                  const int centerX,
                                  const int centerY,
                                  const int radius,
                                  const precision sub_pixel_precision,
                                  const opacity_t opacity) {
    color_t color_int;
    const uint8_t p = sub_pixel_precision;
    coder().convert(color, color_int);
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
                if(apply_opacity) blend = (blend*opacity)>>8;
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
                                  opacity_t opacity) {
    const precision p = 4;
    drawCircle<BlendMode, PorterDuff, antialias>(color,
                microgl::math::to_fixed(centerX, p),
                microgl::math::to_fixed(centerY, p),
                microgl::math::to_fixed(radius, p),
                p, opacity);
}

// Triangles

template<typename P, typename CODER>
template<typename iterator_callback>
void Canvas<P, CODER>::iterate_triangles(const index *indices,
                                         const index &size,
                                         const enum triangles::indices &type,
                                         const iterator_callback && callback) {
#define IND(a) indices[(a)]
    switch (type) {
        case indices::TRIANGLES:
        case indices::TRIANGLES_WITH_BOUNDARY:
            for (index ix = 0, idx=0; ix < size; ix+=3,idx++)
                callback(idx, IND(ix + 0), IND(ix + 1), IND(ix + 2));
            break;
        case indices::TRIANGLES_FAN:
        case indices::TRIANGLES_FAN_WITH_BOUNDARY:
            for (index ix = 1; ix < size-1; ++ix)
                callback(ix-1, IND(0), IND(ix), IND(ix + 1));
            break;
        case indices::TRIANGLES_STRIP:
        case indices::TRIANGLES_STRIP_WITH_BOUNDARY:
        {
            bool even = true;
            for (index ix = 0; ix < size-2; ++ix) {
                // we alternate order inorder to preserve CCW or CW,
                index first_index = even ?  IND(ix + 0) : IND(ix + 2);
                index second_index = IND(ix + 1);
                index third_index = even ?  IND(ix + 2) : IND(ix + 0);
                callback(ix, first_index, second_index, third_index);
                even = !even;
            }
            break;
        }
    }
#undef IND
}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void Canvas<P, CODER>::drawTriangles(const color_f_t &color,
                                     const vec2<number> *vertices,
                                     const index *indices,
                                     const boundary_info * boundary_buffer,
                                     const index size,
                                     const enum indices type,
                                     const opacity_t opacity) {
#define to_fixed microgl::math::to_fixed
    const precision p = 4;
    iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
                      [&](const index &idx, const index &first_index, const index &second_index, const index &third_index) {
                          const bool aa_2d= boundary_buffer!=nullptr;
                          bool aa_first_edge=true, aa_second_edge=true, aa_third_edge=true;
                          if(aa_2d) {
                              boundary_info aa_info = boundary_buffer[idx];
                              aa_first_edge = triangles::classify_boundary_info(aa_info, 0);
                              aa_second_edge = triangles::classify_boundary_info(aa_info, 1);
                              aa_third_edge = triangles::classify_boundary_info(aa_info, 2);
                          }
                          drawTriangle<BlendMode, PorterDuff, antialias>(color,
                                                 to_fixed(vertices[first_index].x, p), to_fixed(vertices[first_index].y, p),
                                                 to_fixed(vertices[second_index].x, p), to_fixed(vertices[second_index].y, p),
                                                 to_fixed(vertices[third_index].x, p), to_fixed(vertices[third_index].y, p),
                                                 opacity, p, aa_first_edge, aa_second_edge, aa_third_edge);
                      });
#undef to_fixed
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
    iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
                      [&](const index &idx, const index &first_index, const index &second_index, const index &third_index) {
                          const bool aa_2d= boundary_buffer!=nullptr;
                          bool aa_first_edge=true, aa_second_edge=true, aa_third_edge=true;
                          if(aa_2d) {
                              boundary_info aa_info = boundary_buffer[idx];
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
    iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
                      [&](const index &idx, const index &first_index, const index &second_index, const index &third_index) {
                          drawTriangleWireframe(color, vertices[first_index], vertices[second_index], vertices[third_index]);
                      });
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
                                    const opacity_t opacity,
                                    const precision sub_pixel_precision,
                                    bool aa_first_edge,
                                    bool aa_second_edge,
                                    bool aa_third_edge) {

    fixed_signed sign = functions::orient2d({v0_x, v0_y},
            {v1_x, v1_y}, {v2_x, v2_y},
            sub_pixel_precision);
    // discard degenerate triangle
    if(sign==0) return;
    // convert CCW to CW triangle
    if(sign<0) {
        functions::swap(v1_x, v2_x);
        functions::swap(v1_y, v2_y);
        functions::swap(aa_first_edge, aa_third_edge);
    }

    color_t color_int;
    coder().convert(color, color_int);
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
                                    const opacity_t opacity,
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
        typename S>
void Canvas<P, CODER>::drawTriangle(const sampling::sampler<S> &sampler,
                                   int v0_x, int v0_y, int u0, int v0, int q0,
                                   int v1_x, int v1_y, int u1, int v1, int q1,
                                   int v2_x, int v2_y, int u2, int v2, int q2,
                                   const opacity_t opacity, const precision sub_pixel_precision, const precision uv_precision,
                                   bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    fixed_signed area = functions::orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
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
            triangles::classifyTopLeftEdges(false, v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);

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
            if ((w0|w1|w2)>=0) {
                int u_i, v_i;
                uint64_t u_fixed = (((uint64_t)((uint64_t)w0*u2 + (uint64_t)w1*u0 + (uint64_t)w2*u1)));
                uint64_t v_fixed = (((uint64_t)((uint64_t)w0*v2 + (uint64_t)w1*v0 + (uint64_t)w2*v1)));

                if(perspective_correct) {
                    uint64_t q_fixed =(((uint64_t)((uint64_t)w0*q2 + (uint64_t)w1*q0 + (uint64_t)w2*q1)));
                    u_i = (u_fixed<<BITS_UV_COORDS)/q_fixed;
                    v_i = (v_fixed<<BITS_UV_COORDS)/q_fixed;
                } else {
                    u_fixed = ((u_fixed*one_area)>>(LL - BITS_UV_COORDS));
                    v_fixed = ((v_fixed*one_area)>>(LL - BITS_UV_COORDS));
                    u_i = (u_fixed)>>(BITS_UV_COORDS);
                    v_i = (v_fixed)>>(BITS_UV_COORDS);
//
//                    u_i = u_fixed/area;
//                    v_i = v_fixed/area;
                }
                color_t col_bmp;
                sampler.sample(u_i, v_i, BITS_UV_COORDS, col_bmp);
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
                    int64_t u_i, v_i;
                    int64_t u_fixed = (((int64_t)((int64_t)w0*u2 + (int64_t)w1*u0 + (int64_t)w2*u1)));
                    int64_t v_fixed = (((int64_t)((int64_t)w0*v2 + (int64_t)w1*v0 + (int64_t)w2*v1)));

                    if(perspective_correct) {
                        int64_t q_fixed =(((int64_t)((int64_t)w0*q2 + (int64_t)w1*q0 + (int64_t)w2*q1)));
                        u_i = ((u_fixed<<BITS_UV_COORDS)/q_fixed);
                        v_i = ((v_fixed<<BITS_UV_COORDS)/q_fixed);
                    } else {
                        u_fixed = ((u_fixed*one_area)>>(LL - BITS_UV_COORDS));
                        v_fixed = ((v_fixed*one_area)>>(LL - BITS_UV_COORDS));
                        // coords in :BITS_UV_COORDS space
                        u_i = int64_t(u_fixed)>>(BITS_UV_COORDS);
                        v_i = int64_t(v_fixed)>>(BITS_UV_COORDS);
                    }
                    u_i = functions::clamp<int64_t>(u_i, 0, 1ll<<BITS_UV_COORDS);
                    v_i = functions::clamp<int64_t>(v_i, 0, 1ll<<BITS_UV_COORDS);

                    color_t col_bmp;
                    sampler.sample(u_i, v_i, BITS_UV_COORDS, col_bmp);
                    // complement and normalize
                    uint8_t blend = functions::clamp<int>(((uint64_t)(delta << bits_distance_complement))>>PREC_DIST,
                                                          0, 255);
                    if (opacity < _max_alpha_value) blend = (blend * opacity) >> 8;
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

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename S, typename number>
void Canvas<P, CODER>::drawTriangle(const sampling::sampler<S> & sampler,
                                   const number v0_x, const number v0_y, number u0, number v0,
                                   const number v1_x, const number v1_y, number u1, number v1,
                                   const number v2_x, const number v2_y, number u2, number v2,
                                   const opacity_t opacity,
                                   bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    precision prec_pixel = 8, prec_uv = 8;
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
    drawTriangle<BlendMode, PorterDuff, antialias, false, S>(sampler,
            v0_x_, v0_y_, u0_, v0_, q_,
            v1_x_, v1_y_, u1_, v1_, q_,
            v2_x_, v2_y_, u2_, v2_, q_,
            opacity, prec_pixel, prec_uv,
            aa_first_edge, aa_second_edge, aa_third_edge);
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
    const bits sub_pixel_precision = 4;
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

    l64 area = functions::orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
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
    // this can produce a 2P bits number if the points form a a perpendicular triangle
    const int area_v1_v2_p = functions::orient2d({v1_x, v1_y}, {v2_x, v2_y}, p_fixed, sub_pixel_precision) + bias_w1;
    const int area_v2_v0_p = functions::orient2d({v2_x, v2_y}, {v0_x, v0_y}, p_fixed, sub_pixel_precision) + bias_w2;
    const int area_v0_v1_p = functions::orient2d({v0_x, v0_y}, {v1_x, v1_y}, p_fixed, sub_pixel_precision) + bias_w0;

    // Triangle setup, this needs at least (P+1) bits, since the delta is always <= length
    int64_t A01 = (v0_y - v1_y), B01 = (v1_x - v0_x);
    int64_t A12 = (v1_y - v2_y), B12 = (v2_x - v1_x);
    int64_t A20 = (v2_y - v0_y), B20 = (v0_x - v2_x);

    int64_t w0_row = (area_v0_v1_p);
    int64_t w1_row = (area_v1_v2_p);
    int64_t w2_row = (area_v2_v0_p);

    // AA, 2A/L = h, therefore the division produces a P bit number
    int64_t w0_row_h=0, w1_row_h=0, w2_row_h=0;
    int64_t A01_h=0, B01_h=0, A12_h=0, B12_h=0, A20_h=0, B20_h=0;

    if(antialias) {
        // lengths of edges, produces a P+1 bits number
        int64_t length_w0 = functions::length({v0_x, v0_y}, {v1_x, v1_y}, 0);
        int64_t length_w1 = functions::length({v1_x, v1_y}, {v2_x, v2_y}, 0);
        int64_t length_w2 = functions::length({v0_x, v0_y}, {v2_x, v2_y}, 0);

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
                // todo: color conversion is missing
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
        bool antialias, typename number, typename S>
void Canvas<P, CODER>::drawQuadrilateral(const sampling::sampler<S> & sampler,
                                    number v0_x, number v0_y, number u0, number v0,
                                    number v1_x, number v1_y, number u1, number v1,
                                    number v2_x, number v2_y, number u2, number v2,
                                    number v3_x, number v3_y, number u3, number v3,
                                    const uint8_t opacity) {
    precision uv_p = 15, pixel_p = 4;
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
template<typename BlendMode, typename PorterDuff>
void Canvas<P, CODER>::drawQuad(const color_f_t & color,
                                const int left, const int top,
                                const int right, const int bottom,
                                const precision sub_pixel_precision,
                                const opacity_t opacity) {
    color_t color_int;
    this->coder().convert(color, color_int);
    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;
    int left_ = functions::max((left + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)0);
    int top_ = functions::max((top + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)0);
    int right_ = functions::min((right + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)width()-1);
    int bottom_ = functions::min((bottom + max_sub_pixel_precision_value) >> sub_pixel_precision, (unsigned int)height()-1);
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
                                const opacity_t opacity) {
    uint8_t p = 4;
    drawQuad<BlendMode, PorterDuff>(color,
             microgl::math::to_fixed(left, p), microgl::math::to_fixed(top, p),
             microgl::math::to_fixed(right, p), microgl::math::to_fixed(bottom, p),
             p, opacity);
}

template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff, typename S,
        typename number>
void Canvas<P, CODER>::drawQuad(const sampling::sampler<S> & sampler,
                                const number left, const number top,
                                const number right, const number bottom,
                                const opacity_t opacity,
                                const number u0, const number v0,
                                const number u1, const number v1) {
    precision p_sub = 4, p_uv = 10;
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
    int max = (1u<<sub_pixel_precision) - 1;
    int left_   = functions::max(left, (int)0);
    int top_    = functions::max(top, ( int)0);
    int right_  = functions::min(right, (width()-1)<<sub_pixel_precision);
    int bottom_ = functions::min(bottom, (height()-1)<<sub_pixel_precision);
    // intersections
    u0 = u0+((u1-u0) *(left_-left))/(right-left);
    v0 = v0+((v1-v0) *(top_-top))/(bottom-top);
    u1 = u0+((u1-u0) *(right_-left))/(right-left);
    v1 = v0+((v1-v0) *(bottom_-top))/(bottom-top);
    // round and convert to raster space
    left_   = (max + left_  )>>sub_pixel_precision;
    top_    = (max + top_   )>>sub_pixel_precision;
    right_  = (max + right_ )>>sub_pixel_precision;
    bottom_ = (max + bottom_)>>sub_pixel_precision;
    // MULTIPLYING with texture dimensions and doubling precision, helps with the z-fighting
    int du = ((u1-u0)<<uv_precision) / (right_ - left_);
    int dv = -((v1-v0)<<uv_precision) / (bottom_ - top_);
    int u_start = u0<<uv_precision;
    int u = u_start;
    int v = -dv*(bottom_ - top_);
    int index = top_ * _width;
    const precision pp = uv_precision<<1;
    for (int y = top_; y <= bottom_; y++) {
        for (int x = left_; x <= right_; x++) {
            sampler.sample(u, v, pp, col_bmp);
            blendColor<BlendMode, PorterDuff>(col_bmp, index + x, opacity);
            u += du;
        }
        u = u_start;
        v += dv;
        index += _width;
    }
}

template<typename P, typename CODER>
template <typename number, typename S>
void Canvas<P, CODER>::drawMask(const masks::chrome_mode &mode,
                                const sampling::sampler<S> & sampler,
                                const number left, const number top,
                                const number right, const number bottom,
                                const number u0, const number v0,
                                const number u1, const number v1,
                                const opacity_t opacity) {
    precision p_sub = 4, p_uv = 8;
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
    u0 = u0+((u1-u0) *(left_-left))/(right-left);
    v0 = v0+((v1-v0) *(top_-top))/(bottom-top);
    u1 = u0+((u1-u0) *(right_-left))/(right-left);
    v1 = v0+((v1-v0) *(bottom_-top))/(bottom-top);
    // round and convert to raster space
    left_   = (max + left_  )>>sub_pixel_precision;
    top_    = (max + top_   )>>sub_pixel_precision;
    right_  = (max + right_ )>>sub_pixel_precision;
    bottom_ = (max + bottom_)>>sub_pixel_precision;
    // increase precision to (uv_precision*2)
    int du = ((u1-u0)<<uv_precision) / (right_ - left_);
    int dv = -((v1-v0)<<uv_precision) / (bottom_ - top_);
    int u_start = u0<<uv_precision;
    int u = u_start;
    int v = -dv*(bottom_ - top_);
    int index = top_ * _width;
    const precision pp = uv_precision<<1;
    const bits alpha_bits = this->coder().alpha_bits() | 8;
    const channel max_alpha_value = (1<<alpha_bits) - 1;
    for (int y = top_; y <= bottom_; y++) {
        for (int x = left_; x <= right_; x++) {
            sampler.sample(u, v, pp, col_bmp);
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
            u += du;
        }
        u = u_start;
        v += dv;
        index += _width;
    }
}

template<typename P, typename CODER>
template<typename number>
void Canvas<P, CODER>::drawLine(const color_f_t &color,
                                number x0, number y0,
                                number x1, number y1) {
    using clipper = microgl::clipping::cohen_sutherland_clipper<number>;
    auto clip =  clipper::compute(x0, y0, x1, y1, number(0), number(0), width(), height());
    if(!clip.inside) return;
    precision p = 4;
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
template<typename BlendMode, typename PorterDuff, bool antialias, typename number, typename S>
void Canvas<P, CODER>::drawBezierPatch(const sampling::sampler<S> & sampler,
                                     const vec3<number> *mesh,
                                     const unsigned U, const unsigned V,
                                     const unsigned uSamples, const unsigned vSamples,
                                     const opacity_t opacity) {
    using tess= microgl::tessellation::bezier_patch_tesselator<number>;
    dynamic_array<number> vertices_attributes;
    dynamic_array<index> indices;
    microgl::triangles::indices indices_type;
    tess::compute(mesh, U, V, uSamples, vSamples, vertices_attributes, indices, indices_type);
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

template<typename P, typename CODER>
template <typename BlendMode,
        typename PorterDuff,
        bool antialias, typename number>
void Canvas<P, CODER>::drawPolygon(vec2<number> *points,
                                   Canvas::index size,
                                   opacity_t opacity,
                                   polygons::hints hint
                                   ) {
    indices type;
    // currently static on the stack
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;

    switch (hint) {
        case hints::CONCAVE:
        case hints::SIMPLE:
        {
            type = antialias ? triangles::indices::TRIANGLES_WITH_BOUNDARY :
                   triangles::indices::TRIANGLES;
            microgl::tessellation::ear_clipping_triangulation<number>::compute(points,
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
            microgl::tessellation::fan_triangulation<number>::compute(points,
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
    // draw triangles batch
    drawTriangles<BlendMode, PorterDuff, antialias>(
            color::colors::RED,
            points,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            opacity);
    drawTrianglesWireframe(color::colors::BLACK,
                           points,
                           indices.data(),
                           indices.size(),
                           type,
                           255);
}