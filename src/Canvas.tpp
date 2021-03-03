
template<typename BITMAP, uint8_t options>
canvas<BITMAP, options>::canvas(bitmap *$bmp) : _bitmap_canvas($bmp) {
    updateCanvasWindow(0, 0, $bmp);
    updateClipRect(0, 0, $bmp->width(), $bmp->height());
}

template<typename BITMAP, uint8_t options>
canvas<BITMAP, options>::canvas(int width, int height) :
            canvas(new bitmap(width, height)) {
}

template<typename BITMAP, uint8_t options>
auto canvas<BITMAP, options>::coder() const -> const pixel_coder & {
    return _bitmap_canvas->coder();
}

template<typename BITMAP, uint8_t options>
inline BITMAP *canvas<BITMAP, options>::bitmapCanvas() const {
    return _bitmap_canvas;
}

template<typename BITMAP, uint8_t options>
unsigned int canvas<BITMAP, options>::sizeofPixel() const {
    return sizeof(pixel{});
}

template<typename BITMAP, uint8_t options>
auto canvas<BITMAP, options>::getPixel(int x, int y) const -> pixel & {
    // this is not good for high performance loop
    return _bitmap_canvas->pixelAt(x, y);
}

template<typename BITMAP, uint8_t options>
auto canvas<BITMAP, options>::getPixel(int index) const -> pixel & {
    // this is better for high performance loop
    return _bitmap_canvas->pixelAt(index);
}

template<typename BITMAP, uint8_t options>
void canvas<BITMAP, options>::getPixelColor(int x, int y, color_t & output)  const {
    this->_bitmap_canvas->decode(x, y, output);
}

template<typename BITMAP, uint8_t options>
void canvas<BITMAP, options>::getPixelColor(int index, color_t & output)  const {
    this->_bitmap_canvas->decode(index - _window.index_correction, output);
}

template<typename BITMAP, uint8_t options>
int canvas<BITMAP, options>::width() const {
    return _window.canvas_rect.width();
}

template<typename BITMAP, uint8_t options>
int canvas<BITMAP, options>::height() const {
    return _window.canvas_rect.height();
}

template<typename BITMAP, uint8_t options>
auto canvas<BITMAP, options>::pixels()  const -> pixel *{
    return _bitmap_canvas->data();
}

template<typename BITMAP, uint8_t options>
template <typename number>
void canvas<BITMAP, options>::clear(const intensity<number> &color) {
    pixel output;
    _bitmap_canvas->coder().encode(color, output);
    _bitmap_canvas->fill(output);
}

template<typename BITMAP, uint8_t options>
void canvas<BITMAP, options>::clear(const color_t &color) {
    pixel output;
    _bitmap_canvas->coder().encode(color, output);
    _bitmap_canvas->fill(output);
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, uint8_t a_src>
inline void canvas<BITMAP, options>::blendColor(const color_t &val, int x, int y, opacity_t opacity) {
    blendColor<BlendMode, PorterDuff, a_src>(val, y*width() + x, opacity);
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, uint8_t a_src>
inline void canvas<BITMAP, options>::blendColor(const color_t &val, int index, opacity_t opacity) {
    // correct index position when window is not at the (0,0) costs one subtraction.
    // we use it for sampling the backdrop if needed and for writing the output pixel
    index -= _window.index_correction;
    // we assume that the color conforms to the same pixel-coder. but we are flexible
    // for alpha channel. if coder does not have an alpha channel, the color itself may
    // have non-zero alpha channel, for which we emulate 8-bit alpha processing and also pre
    // multiply result with alpha
    constexpr bool hasBackdropAlphaChannel = pixel_coder::rgba::a != 0;
    constexpr bool hasSrcAlphaChannel = a_src != 0;
    constexpr uint8_t canvas_a_bits = hasBackdropAlphaChannel ? pixel_coder::rgba::a : (a_src ? a_src : 8);
    constexpr uint8_t src_a_bits = a_src ? a_src : 8;
    constexpr uint8_t alpha_bits = src_a_bits;
    constexpr unsigned int alpha_max_value = uint16_t (1 << alpha_bits) - 1;
    constexpr bool is_source_over = microgl::traits::is_same<PorterDuff, porterduff::FastSourceOverOnOpaque>::value;
    constexpr bool none_compositing = microgl::traits::is_same<PorterDuff, porterduff::None<>>::value;
    constexpr bool skip_blending =microgl::traits::is_same<BlendMode, blendmode::Normal>::value;
    constexpr bool premultiply_result = !hasBackdropAlphaChannel;
    const bool skip_all= skip_blending && none_compositing && opacity == 255;
    static_assert(src_a_bits==canvas_a_bits, "src_a_bits!=canvas_a_bits");

    const color_t & src = val;
    color_t result;
    pixel output;

    if(!skip_all) {
        color_t backdrop, blended;
        // normal blend and none composite do not require a backdrop
        if(!(skip_blending && none_compositing))
            this->_bitmap_canvas->decode(index, backdrop); // not using getPixelColor to avoid extra subtraction

        // support compositing even if the surface is opaque.
        if(!hasBackdropAlphaChannel) backdrop.a = alpha_max_value;

        if(is_source_over && src.a==0) return;

        // if we are normal then do nothing
        if(!skip_blending && backdrop.a!=0) { //  or backdrop alpha is zero is also valid
            BlendMode::template blend<pixel_coder::rgba::r,
                                      pixel_coder::rgba::g,
                                      pixel_coder::rgba::b>(backdrop, src, blended);
//                             pixel_coder::rgba::r,
//                             pixel_coder::rgba::g,
//                             pixel_coder::rgba::b);
            // if backdrop alpha!= max_alpha let's first composite the blended color, this is
            // an intermediate step before Porter-Duff
            if(backdrop.a < alpha_max_value) {
                // if((backdrop.a ^ _max_alpha_value)) {
                unsigned int comp = alpha_max_value - backdrop.a;
                // this is of-course a not accurate interpolation, we should
                // divide by 255. bit shifting is like dividing by 256 and is FASTER.
                // you will pay a price when bit count is low, this is where the error
                // is very noticeable.
                blended.r = (comp * src.r + backdrop.a * blended.r) >> alpha_bits;
                blended.g = (comp * src.g + backdrop.a * blended.g) >> alpha_bits;
                blended.b = (comp * src.b + backdrop.a * blended.b) >> alpha_bits;
            }
        }
        else {
            // skipped blending therefore use src color
            blended.r = src.r; blended.g = src.g; blended.b = src.b;
        }

        // support alpha channel in case, source pixel does not have
        blended.a = hasSrcAlphaChannel ? src.a : alpha_max_value;

        // I fixed opacity is always 8 bits no matter what the alpha depth of the native canvas
        if(opacity < 255) blended.a =  (int(blended.a) * int(opacity)*int(257) + 257)>>16; // blinn method

        // finally alpha composite with Porter-Duff equations,
        // this should be zero-cost for None option with compiler optimizations
        // if we do not own a native alpha channel, then please keep the composited result
        // with premultiplied alpha, this is why we composite for None option, because it performs
        // alpha multiplication
        PorterDuff::template composite<alpha_bits, premultiply_result>(backdrop, blended, result);
    } else
        result = src;

    coder().encode(result, output);
    _bitmap_canvas->writeAt(index, output);
}

//template<typename bitmap_, uint8_t options>
//template<typename BlendMode, typename PorterDuff>
//inline void canvas<bitmap_, options>::blendColor(const color_t &val, int index, opacity_t opacity) {
//    // correct index position when window is not at the (0,0) costs one subtraction.
//    // we use it for sampling the backdrop if needed and for writing the output pixel
//    index -= _window.index_correction;
//    // we assume that the color conforms to the same pixel-coder. but we are flexible
//    // for alpha channel. if coder does not have an alpha channel, the color itself may
//    // have non-zero alpha channel, for which we emulate 8-bit alpha processing and also pre
//    // multiply result with alpha
//    color_t result;
//    pixel_ output;
//    constexpr bool is_source_over = microgl::traits::is_same<PorterDuff, porterduff::FastSourceOverOnOpaque>::value;
//    constexpr bool none_compositing = microgl::traits::is_same<PorterDuff, porterduff::None<>>::value;
//    constexpr bool skip_blending =microgl::traits::is_same<BlendMode, blendmode::Normal>::value;
//    const bool skip_all= skip_blending && none_compositing && opacity == 255;
//    if(!skip_all) {
//        color_t backdrop, blended;
//        const color_t & src = val;
//        // get backdrop color if blending or compositing is required
//        if(!skip_blending || !none_compositing)
//            this->_bitmap_canvas->decode(index, backdrop); // not using getPixelColor to avoid extra subtraction
//        //getPixelColor(index, backdrop);
//        // we assume that either they are the same or one of them is zero, this is FASTER then comparison.
//        // if we don't own a native alpha channel, check if the color has a suggestion for alpha channel.
//        // I use it because of enabling masks for a canvas that does not have an alpha channel.
//        // I really want to get rid of this
//        bits alpha_bits = pixel_coder::a | val.a_bits;
//        if(alpha_bits) blended.a = src.a;
//        else { blended.a= 255; alpha_bits=8; } // no alpha channel ? let's create one with 8 bits
//        if(is_source_over && blended.a==0) return;
//        constexpr bool hasNativeAlphaChannel = pixel_coder::a != 0;
//        constexpr unsigned int max_alpha_value = hasNativeAlphaChannel ? (1 << pixel_coder::a) - 1 : (255);
//        // fix alpha bits depth in case we don't natively
//        // support alpha, this is correct because we want to
//        // support compositing even if the surface is opaque.
//        if(!hasNativeAlphaChannel) backdrop.a = max_alpha_value;
//        // if blend-mode is normal or the backdrop is completely transparent
//        // then we don't need to blend.
//        // the first conditional should be resolved at compile-time therefore it is zero cost !!!
//        // this will help with avoiding the inner conditional of the backdrop alpha, the normal
//        // blending itself is zero-cost itself, but after it there is a branching which
//        // is unpredictable, therefore avoiding at compile-time is great.
//
//        // if we are normal then do nothing
//        if(!skip_blending && backdrop.a!=0) { //  or backdrop alpha is zero is also valid
//            BlendMode::blend(backdrop, src, blended,
//                             coder().r,
//                             coder().g,
//                             coder().b);
//            // if backdrop alpha!= max_alpha let's first composite the blended color, this is
//            // an intermidiate step before Porter-Duff
//            if(backdrop.a < max_alpha_value) {
//                // if((backdrop.a ^ _max_alpha_value)) {
//                int max_alpha = max_alpha_value;
//                unsigned int comp = max_alpha - backdrop.a;
//                // this is of-course a not accurate interpolation, we should
//                // divide by 255. bit shifting is like dividing by 256 and is FASTER.
//                // you will pay a price when bit count is low, this is where the error
//                // is very noticeable.
//                blended.r = (comp * src.r + backdrop.a * blended.r) >> alpha_bits;
//                blended.g = (comp * src.g + backdrop.a * blended.g) >> alpha_bits;
//                blended.b = (comp * src.b + backdrop.a * blended.b) >> alpha_bits;
//            }
//        }
//        else {
//            // skipped blending therefore use src color
//            blended.r = src.r; blended.g = src.g; blended.b = src.b;
//        }
//        // I fixed opacity is always 8 bits no matter what the alpha depth of the native canvas
//        if(opacity < 255) blended.a =  (int(blended.a) * int(opacity)*int(257))>>16; // blinn method
//        constexpr bool premultiply_result = !hasNativeAlphaChannel;
//        // finally alpha composite with Porter-Duff equations,
//        // this should be zero-cost for None option with compiler optimizations
//        // if we do not own a native alpha channel, then please keep the composited result
//        // with premultiplied alpha, this is why we composite for None option, because it performs
//        // alpha multiplication
//        PorterDuff::template composite<premultiply_result>(backdrop, blended, result, alpha_bits);
//    } else
//        result = val;
//
//    coder().encode(result, output);
//    _bitmap_canvas->writeAt(index, output);
//}

template<typename BITMAP, uint8_t options>
void canvas<BITMAP, options>::drawPixel(const pixel & val, int x, int y) {
    _bitmap_canvas->writeAt(y*width()+x, val);
}

template<typename BITMAP, uint8_t options>
inline void canvas<BITMAP, options>::drawPixel(const pixel & val, int index) {
    _bitmap_canvas->writeAt(index - _window.index_correction, val);
}

// fast common graphics shapes like circles and rounded rectangles

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias,
        typename number1, typename number2, typename S1, typename S2>
void canvas<BITMAP, options>::drawCircle(const sampler<S1> & sampler_fill,
                                         const sampler<S2> & sampler_stroke,
                                         const number1 &centerX, const number1 &centerY,
                                         const number1 &radius, const number1 &stroke_size, opacity_t opacity,
                                         const number2 &u0, const number2 &v0,
                                         const number2 &u1, const number2 &v1) {
    drawRoundedRect<BlendMode, PorterDuff, antialias, number1, number2, S1, S2>(sampler_fill, sampler_stroke,
                                                                                centerX - radius, centerY - radius,
                                                                                centerX + radius, centerY + radius,
                                                                                radius, stroke_size, opacity,
                                                                                u0, v0, u1, v1);
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S1, typename S2>
void canvas<BITMAP, options>::drawRoundedRect(const sampler<S1> & sampler_fill,
                                              const sampler<S2> & sampler_stroke,
                                              const number1 & left, const number1 & top,
                                              const number1 & right, const number1 & bottom,
                                              const number1 & radius, const number1 & stroke_size,
                                              canvas::opacity_t opacity,
                                              const number2 & u0, const number2 & v0,
                                              const number2 & u1, const number2 & v1) {
    const precision p = renderingOptions()._2d_raster_bits_sub_pixel;
    const precision p_uv = renderingOptions()._2d_raster_bits_uv;
#define f_p(x) microgl::math::to_fixed((x), p)
#define f_uv(x) microgl::math::to_fixed((x), p_uv)
    drawRoundedRect<BlendMode, PorterDuff, antialias>(sampler_fill, sampler_stroke, f_p(left), f_p(top), f_p(right),
            f_p(bottom), f_p(radius), f_p(stroke_size), f_uv(u0), f_uv(v0), f_uv(u1), f_uv(v1), p, p_uv, opacity);
#undef f_uv
#undef f_p
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, typename S1, typename S2>
void canvas<BITMAP, options>::drawRoundedRect(const sampler<S1> & sampler_fill,
                                              const sampler<S2> & sampler_stroke,
                                              int left, int top,
                                              int right, int bottom,
                                              int radius, int stroke_size,
                                              int u0, int v0, int u1, int v1,
                                              precision sub_pixel_precision, precision uv_p,
                                              canvas::opacity_t opacity) {
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
    const precision p = sub_pixel_precision;
    const rint step = (rint(1)<<p);
    const rint half = step>>1;
    const rint stroke = stroke_size-step;//(10<<p)/1;
    const rint aa_range = step;// (1<<p)/1;
    const rint radius_squared=(rint_big(radius)*(radius))>>p;
    const rint stroke_radius = (rint_big(radius-(stroke-0))*(radius-(stroke-0)))>>p;
    const rint outer_aa_radius = (rint_big(radius+aa_range)*(radius+aa_range))>>p;
    const rint outer_aa_bend = outer_aa_radius-radius_squared;
    const rint inner_aa_radius = (rint_big(radius-(stroke-0)-aa_range)*rint_big(radius-(stroke-0)-aa_range))>>p;
    const rint inner_aa_bend = stroke_radius-inner_aa_radius;
    const bool apply_opacity = opacity!=255;
    const rint mask= (rint(1)<<sub_pixel_precision)-1;
    // dimensions in two spaces, one in raster spaces for optimization
    const rint left_=(left+0), top_=(top+0), right_=(right), bottom_=(bottom);
    const rect bbox_r = {left_>>p, top_>>p,(right_+aa_range)>>p, (bottom_+aa_range)>>p};
    const rect bbox_r_c = bbox_r.intersect(effectiveRect);
    if(bbox_r_c.empty()) return;
    // calculate uvs with original unclipped deltas, this way we can always accurately predict blocks
    const auto bits_du=microgl::functions::used_integer_bits(u1-u0);
    const auto bits_dv=microgl::functions::used_integer_bits(v1-v0);
    const auto bits_width=microgl::functions::used_integer_bits(bbox_r.width());
    const auto bits_height=microgl::functions::used_integer_bits(bbox_r.height());
    // boost is done against compressed sub pixel coords, so we really cannot overflow, we leave
    // (31-14=17) bits for coords without sub-pixel
    const precision boost_bits=14, boost_width= bits_width + boost_bits, boost_height= bits_height + boost_bits;
    precision boost_u=0, boost_v=0;
    if(boost_width > bits_du) boost_u= boost_width - bits_du;
    if(boost_height > bits_dv) boost_v= boost_height - bits_dv;
    u0=u0<<boost_u;v0=v0<<boost_v;u1=u1<<boost_u;v1=v1<<boost_v; // this is (coords-sub_pixel bits)+(boost_bits=14) bits
    const int du= (u1-u0)/(bbox_r.width()); // this occupies (boost_bits=14) bits
    const int dv = (v1-v0)/(bbox_r.height()); // this occupies (boost_bits=14) bits
    const rint dx=bbox_r_c.left-bbox_r.left, dy=bbox_r_c.top-bbox_r.top;
    color_t color;
    const int pitch = width();
    int index = bbox_r_c.top * pitch;
    for (rint y_r=bbox_r_c.top, yy=(top_&~mask)+dy*step, v=v0+dy*dv+(dv>>1); y_r<=bbox_r_c.bottom; y_r++, yy+=step, v+=dv, index+=pitch) {
        for (rint x_r=bbox_r_c.left, xx=(left_&~mask)+dx*step, u=u0+dx*du+(du>>1); x_r<=bbox_r_c.right; x_r++, xx+=step, u+=du) {
            int blend_fill=opacity, blend_stroke=opacity;
            bool inside_radius;
            bool sample_fill=true, sample_stroke=false;
            const bool in_top_left= xx<=left_+radius && yy<=top_+radius;
            const bool in_bottom_left= xx<=left_+radius && yy>=bottom_-radius;
            const bool in_top_right= xx>=right_-radius && yy<=top_+radius;
            const bool in_bottom_right= xx>=right_-radius && yy>=bottom_-radius;
            const bool in_disks= in_top_left || in_bottom_left || in_top_right || in_bottom_right;
            if(in_disks) {
                rint anchor_x=0, anchor_y=0;
                if(in_top_left) {anchor_x= left_+radius, anchor_y=top_+radius; }
                if(in_bottom_left) {anchor_x= left_+radius, anchor_y=bottom_-radius; }
                if(in_top_right) {anchor_x= right_-radius, anchor_y=top_+radius; }
                if(in_bottom_right) {anchor_x= right_-radius, anchor_y=bottom_-radius; }

                rint delta_x = xx - anchor_x, delta_y = yy - anchor_y;
                const rint distance_squared = ((rint(delta_x) * delta_x) >> p) + ((rint(delta_y) * delta_y) >> p);
                sample_fill=inside_radius = (distance_squared - radius_squared) <= 0;

                if (inside_radius) {
                    const bool inside_stroke = (distance_squared - stroke_radius) >= 0;
                    if (inside_stroke) { // inside stroke disk
                        blend_stroke = opacity;
                        sample_stroke=true;
                    }
                    else { // outside stroke disk, let's test_texture for aa disk or radius inclusion
                        const rint delta_inner_aa = -inner_aa_radius + distance_squared;
                        const bool inside_inner_aa_ring = delta_inner_aa >= 0;
                        if (antialias && inside_inner_aa_ring) {
                            // scale inner to 8 bit and then convert to integer
                            blend_stroke = ((delta_inner_aa) << (8)) / inner_aa_bend;
                            if (apply_opacity) blend_stroke = (blend_stroke * opacity) >> 8;
                            sample_stroke=true;
                        }
                    }
                } else if (antialias) { // we are outside the main radius
                    const int delta_outer_aa = outer_aa_radius - distance_squared;
                    const bool inside_outer_aa_ring = delta_outer_aa >= 0;
                    if (inside_outer_aa_ring) {
                        // scale inner to 8 bit and then convert to integer
                        blend_stroke = ((delta_outer_aa) << (8)) / outer_aa_bend;
                        if (apply_opacity) blend_stroke = (blend_stroke * opacity) >> 8;
                        sample_stroke=true;
                    }
                }
            } else {
                // are we in external AA region ?
                // few notes:
                // this is not an accurate AA rounded rectangle, I use tricks to speed things up.
                // - I need to do AA for the stroke walls, calculate the coverage.
                //   currently I don't, therefore, there is a step function when rounded rectangle
                //   moves in sub pixel coords
                if(xx>right_ || yy>bottom_) {
                    sample_fill=sample_stroke=false;
                } else {
                    if(xx-(left_&~mask)+0<=stroke) sample_stroke=true;
                    else if(xx>=((right_&~mask) -stroke)) sample_stroke=true;
                    else if(yy-(top_&~mask)<=stroke) sample_stroke=true;
                    else if(yy>=(bottom_&~mask)-stroke) sample_stroke=true;
                    else sample_stroke=false;
                    sample_fill=true;
                }
            }
            if (sample_fill) {
                sampler_fill.sample(u>>boost_u, v>>boost_v, uv_p, color);
                blendColor<BlendMode, PorterDuff, S1::a>(color, (index+x_r), blend_fill);
            }
            if (sample_stroke) {
                sampler_stroke.sample(u>>boost_u, v>>boost_v, uv_p, color);
                blendColor<BlendMode, PorterDuff, S2::a>(color, (index+x_r), blend_stroke);
            }
        }
    }
}

template<typename BITMAP, uint8_t options>
template <typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawRect(const sampler<S> & sampler,
                                       const number1 left, const number1 top,
                                       const number1 right, const number1 bottom,
                                       opacity_t opacity,
                                       const number2 u0, const number2 v0,
                                       const number2 u1, const number2 v1) {
    const precision p_sub = renderingOptions()._2d_raster_bits_sub_pixel,
            p_uv = renderingOptions()._2d_raster_bits_uv;
    drawRect<BlendMode, PorterDuff, antialias, S > (sampler,
            microgl::math::to_fixed(left, p_sub), microgl::math::to_fixed(top, p_sub),
            microgl::math::to_fixed(right, p_sub), microgl::math::to_fixed(bottom, p_sub),
            microgl::math::to_fixed(u0, p_uv), microgl::math::to_fixed(v0, p_uv),
            microgl::math::to_fixed(u1, p_uv), microgl::math::to_fixed(v1, p_uv),
            p_sub, p_uv, opacity);
}

template<typename BITMAP, uint8_t options>
template <typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawRect(const sampler<S> & sampler,
                                       const matrix_3x3<number1> &transform,
                                       const number1 left, const number1 top,
                                       const number1 right, const number1 bottom,
                                       opacity_t opacity,
                                       const number2 u0, const number2 v0,
                                       const number2 u1, const number2 v1) {
    vec2<number1> p0{left, top}, p1{left, bottom}, p2{right, bottom}, p3{right, top};
    if(!transform.isIdentity()) {p0=transform*p0; p1=transform*p1; p2=transform*p2; p3=transform*p3;}
    drawTriangle<BlendMode, PorterDuff, antialias, number1, number2, S>(sampler,
                                                                        p0.x,p0.y,u0,v0, p1.x,p1.y,u0,v1, p2.x,p2.y,u1,v1, opacity, true, true, false);
    drawTriangle<BlendMode, PorterDuff, antialias, number1, number2, S>(sampler,
                                                                        p2.x,p2.y,u1,v1, p3.x,p3.y,u1,v0, p0.x,p0.y,u0,v0, opacity, true, true, false);
}

template<typename BITMAP, uint8_t options>
template <typename BlendMode, typename PorterDuff, bool antialias, typename S>
void canvas<BITMAP, options>::drawRect(const sampler<S> & sampler,
                                       int left, int top,
                                       int right, int bottom,
                                       int u0, int v0,
                                       int u1, int v1,
                                       precision sub_pixel_precision,
                                       precision uv_precision,
                                       opacity_t opacity) {
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
#define ceil_fixed(val, bits) ((val)&((1<<bits)-1) ? ((val>>bits)+1) : (val>>bits))
#define floor_fixed(val, bits) (val>>bits)
    color_t col_bmp{};
    const precision p= sub_pixel_precision;
    if(left==right || top==bottom) return;
    const rect bbox_r = {floor_fixed(left, p), floor_fixed(top, p),
                         ceil_fixed(right, p)-0, ceil_fixed(bottom, p)-0};
    const rect bbox_r_c = bbox_r.intersect(effectiveRect);
    if(bbox_r_c.empty()) return;
    // calculate uvs with original unclipped deltas, this way we can always accurately predict blocks
    const auto bits_du=microgl::functions::used_integer_bits(u1-u0);
    const auto bits_dv=microgl::functions::used_integer_bits(v1-v0);
    const auto bits_dx=microgl::functions::used_integer_bits(bbox_r.right-bbox_r.left);
    const auto bits_dy=microgl::functions::used_integer_bits(bbox_r.bottom-bbox_r.top);
    // boost is done against compressed sub pixel coords, so we really cannot overflow, we leave
    // (31-14=17) bits for coords without sub-pixel
    const precision boost_bits=14, boost_dx=bits_dx+boost_bits, boost_dy=bits_dy+boost_bits;
    precision boost_u=0, boost_v=0;
    if(boost_dx>bits_du) boost_u=boost_dx-bits_du;
    if(boost_dy>bits_dv) boost_v=boost_dy-bits_dv;
    u0=u0<<boost_u;v0=v0<<boost_v;u1=u1<<boost_u;v1=v1<<boost_v; // this is (coords-sub_pixel bits)+(boost_bits=14) bits
    const int du = (u1-u0)/(bbox_r.right-bbox_r.left); // this occupies (boost_bits=14) bits
    const int dv = (v1-v0)/(bbox_r.bottom-bbox_r.top); // this occupies (boost_bits=14) bits
    const int dx= bbox_r_c.left-bbox_r.left, dy= bbox_r_c.top-bbox_r.top;
    const int pitch= width();
    if(antialias) {
        const bool clipped_left=bbox_r.left!=bbox_r_c.left, clipped_top=bbox_r.top!=bbox_r_c.top;
        const bool clipped_right=bbox_r.right!=bbox_r_c.right, clipped_bottom=bbox_r.bottom!=bbox_r_c.bottom;
        const int max=1<<p, mask=max-1;
        const int coverage_left= max-(left&mask), coverage_right=max-(((bbox_r.right+1)<<p)-right);
        const int coverage_top= max-(top&mask), coverage_bottom=max-(((bbox_r.bottom+1)<<p)-bottom);
        const int blend_left_top= (int(opacity)*((coverage_left*coverage_top)>>p))>>p;
        const int blend_left_bottom= (int(opacity)*((coverage_left*coverage_bottom)>>p))>>p;
        const int blend_right_top= (int(opacity)*((coverage_right*coverage_top)>>p))>>p;
        const int blend_right_bottom=(int(opacity)*((coverage_right*coverage_bottom)>>p))>>p;
        const int blend_left= (int(opacity)*coverage_left)>>p;
        const int blend_top= (int(opacity)*coverage_top)>>p;
        const int blend_right= (int(opacity)*coverage_right)>>p;
        const int blend_bottom= (int(opacity)*coverage_bottom)>>p;
        int index= (bbox_r_c.top) * pitch;
        opacity_t blend=0;
        for (int y=bbox_r_c.top, v=v0+(dv>>1)+dy*dv; y<=bbox_r_c.bottom; y++, v+=dv, index+=pitch) {
            for (int x=bbox_r_c.left, u=u0+(du>>1)+dx*du; x<=bbox_r_c.right; x++, u+=du) {
                blend=opacity;
                if(x==bbox_r_c.left && !clipped_left) {
                    if(y==bbox_r_c.top && !clipped_top) blend= blend_left_top;
                    else if(y==bbox_r_c.bottom && !clipped_bottom) blend= blend_left_bottom;
                    else blend= blend_left;
                }
                else if(x==bbox_r_c.right && !clipped_right) {
                    if(y==bbox_r_c.top && !clipped_top) blend= blend_right_top;
                    else if(y==bbox_r_c.bottom && !clipped_bottom) blend= blend_right_bottom;
                    else blend= blend_right;
                }
                else if(y==bbox_r_c.top && !clipped_top) blend= blend_top;
                else if(y==bbox_r_c.bottom && !clipped_bottom) blend= blend_bottom;
                sampler.sample(u>>boost_u, v>>boost_v, uv_precision, col_bmp);
                blendColor<BlendMode, PorterDuff, S::rgba::a>(col_bmp, index + x, blend);
            }
        }
    }
    else {
        int index= bbox_r_c.top * pitch;
        for (int y=bbox_r_c.top, v=v0+(dv>>1)+dy*dv; y<bbox_r_c.bottom; y++, v+=dv, index+=pitch) {
            for (int x=bbox_r_c.left, u=u0+(du>>1)+dx*du; x<bbox_r_c.right; x++, u+=du) {
                sampler.sample(u>>boost_u, v>>boost_v, uv_precision, col_bmp);
                blendColor<BlendMode, PorterDuff, S::rgba::a>(col_bmp, index + x, opacity);
            }
        }
    }
#undef ceil_fixed
#undef floor_fixed
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff,
        bool antialias, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawQuadrilateral(const sampler<S> & sampler,
                                                const number1 & v0_x, const number1 & v0_y, const number2 & u0, const number2 & v0,
                                                const number1 & v1_x, const number1 & v1_y, const number2 & u1, const number2 & v1,
                                                const number1 & v2_x, const number1 & v2_y, const number2 & u2, const number2 & v2,
                                                const number1 & v3_x, const number1 & v3_y, const number2 & u3, const number2 & v3,
                                                const uint8_t opacity) {
    const precision uv_p = renderingOptions()._2d_raster_bits_uv, pixel_p = renderingOptions()._2d_raster_bits_sub_pixel;
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

// Triangles

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawTriangles(const sampler<S> &sampler,
                                            const matrix_3x3<number1> &transform,
                                            const vec2<number1> *vertices,
                                            const vec2<number2> *uvs,
                                            const index *indices,
                                            const boundary_info * boundary_buffer,
                                            const index size,
                                            const enum indices type,
                                            const opacity_t opacity,
                                            const number2 &u0, const number2 &v0,
                                            const number2 &u1, const number2 &v1) {
#define f microgl::math::to_fixed
    const precision p = renderingOptions()._2d_raster_bits_sub_pixel;
    const precision uv_p = renderingOptions()._2d_raster_bits_uv;
    vec2<number1> min, max;
    if(!uvs) { // if we don't have per-vertex uv, then let's compute
        min.x=max.x=vertices[0].x;
        min.y=max.y=vertices[0].y;
        for (unsigned ix = 0; ix < size; ++ix) { // compute bounding box
            const auto & pt = indices ? vertices[indices[ix]] : vertices[ix];
            if(pt.x<min.x) min.x=pt.x; if(pt.y < min.y) min.y=pt.y;
            if(pt.x>max.x) max.x=pt.x; if(pt.y > max.y) max.y=pt.y;
        }
    }
    triangles::iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
          [&](const index &idx, const index &first_index, const index &second_index, const index &third_index,
              const index &edge_0_id, const index &edge_1_id, const index &edge_2_id) {
              const bool aa_2d= boundary_buffer!=nullptr;
              bool aa_first_edge=true, aa_second_edge=true, aa_third_edge=true;
              if(aa_2d) {
                  const boundary_info aa_info = boundary_buffer[idx];
                  aa_first_edge = triangles::classify_boundary_info(aa_info, edge_0_id);
                  aa_second_edge = triangles::classify_boundary_info(aa_info, edge_1_id);
                  aa_third_edge = triangles::classify_boundary_info(aa_info, edge_2_id);
              }
              auto p1= vertices[first_index], p2=vertices[second_index], p3=vertices[third_index];
              vec2<number2> uv_s{u0,v0}, uv_e{u1,v1}, uv_d{u1-u0,v1-v0};
              auto uv1= uvs?uvs[first_index] : vec2<number2>(p1-min)/vec2<number2>(max-min);
              auto uv2= uvs?uvs[second_index] : vec2<number2>(p2-min)/vec2<number2>(max-min);
              auto uv3= uvs?uvs[third_index] : vec2<number2>(p3-min)/vec2<number2>(max-min);
              uv1= uv_s+uv1*uv_d, uv2= uv_s+uv2*uv_d, uv3= uv_s+uv3*uv_d;
              p1=transform*p1;p2=transform*p2;p3=transform*p3;
              drawTriangle<BlendMode, PorterDuff, antialias, false, S>(sampler,
                      f(p1.x,p), f(p1.y,p), f(uv1.x, uv_p), f(uv1.y, uv_p), 0,
                      f(p2.x,p), f(p2.y,p), f(uv2.x, uv_p), f(uv2.y, uv_p), 0,
                      f(p3.x,p), f(p3.y,p), f(uv3.x, uv_p), f(uv3.y, uv_p), 0,
                      opacity, p, uv_p, aa_first_edge, aa_second_edge, aa_third_edge);
          });
#undef f
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number, typename depth_buffer_type>
void canvas<BITMAP, options>::drawTriangles(shader<impl, vertex_attr, varying, number> &shader,
                                            int viewport_width, int viewport_height,
                                            const vertex_attr *vertex_buffer,
                                            const index *indices,
                                            const index size,
                                            const enum indices type,
                                            const triangles::face_culling & culling,
                                            depth_buffer_type *depth_buffer, const opacity_t opacity,
                                            const number& depth_range_near, const number& depth_range_far) {
    triangles::iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
          [&](const index &idx, const index &first_index, const index &second_index, const index &third_index,
              const index &edge_0_id, const index &edge_1_id, const index &edge_2_id) {
              drawTriangle<BlendMode, PorterDuff, antialias, perspective_correct, depth_buffer_flag>(
                      shader, viewport_width, viewport_height,
                      vertex_buffer[first_index],
                      vertex_buffer[second_index],
                      vertex_buffer[third_index],
                      opacity, culling, depth_buffer, depth_range_near, depth_range_far);
          });
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number>
void canvas<BITMAP, options>::drawTrianglesWireframe(const color_t &color,
                                                     const matrix_3x3<number> &transform,
                                                     const vec2<number> *vertices,
                                                     const index *indices,
                                                     const index size,
                                                     const enum indices type,
                                                     const opacity_t opacity) {
    triangles::iterate_triangles(indices, size, type, // we use lambda because of it's capturing capabilities
              [&](const index &idx, const index &first_index, const index &second_index, const index &third_index,
                  const index &edge_0_id, const index &edge_1_id, const index &edge_2_id) {
                  drawTriangleWireframe(color, transform*vertices[first_index], transform*vertices[second_index],
                          transform*vertices[third_index], opacity);
              });
}

template<typename BITMAP, uint8_t options>
template<typename number>
void canvas<BITMAP, options>::drawTriangleWireframe(const color_t &color,
                                                    const vec2<number> &p0,
                                                    const vec2<number> &p1,
                                                    const vec2<number> &p2,
                                                    const opacity_t opacity) {
    drawWuLine(color, p0.x, p0.y, p1.x, p1.y, opacity);
    drawWuLine(color, p1.x, p1.y, p2.x, p2.y, opacity);
    drawWuLine(color, p2.x, p2.y, p0.x, p0.y, opacity);
}


template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, typename S>
void canvas<BITMAP, options>::drawTriangle(const sampler<S> &sampler,
                                           int v0_x, int v0_y, int u0, int v0, int q0,
                                           int v1_x, int v1_y, int u1, int v1, int q1,
                                           int v2_x, int v2_y, int u2, int v2, int q2,
                                           const opacity_t opacity, const precision sub_pixel_precision,
                                           const precision uv_precision, bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    constexpr precision precision_one_over_area=15;
    constexpr precision P_AA = 16;
    constexpr bool divide=options_use_division(); // compile time flag
    constexpr bool avoid_overflows=options_avoid_overflow(); // compile time flag
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
    rint area_ = functions::orient2d<int, rint_big>(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, sub_pixel_precision);
    if(area_==0) return;
    if(area_<0) { // convert CCW to CW triangle
        area_=-area_;
        functions::swap(v1_x, v2_x); functions::swap(v1_y, v2_y);
        functions::swap(u1, u2); functions::swap(v1, v2);
        functions::swap(q1, q2); functions::swap(aa_first_edge, aa_third_edge);
    }
    const rint area = area_;
    const rint area_c = area>>sub_pixel_precision;
    if(area_c==0) return;

    precision bits_used_area=microgl::functions::used_integer_bits(area);
    precision bits_used_max_uv=
            microgl::functions::used_integer_bits(
                    microgl::functions::abs_max({u0,v0,q0, u1,v1,q1, u2,v2,q2}));
    const precision LL = bits_used_area + precision_one_over_area;
    rint one_area = (rint_big(1)<<LL) / rint_big(area);
    if(avoid_overflows) {
        precision size_of_int_bits = sizeof(rint)<<3, size_of_big_int_bits = sizeof(rint_big)<<3;
        const bool first_test = bits_used_area + bits_used_max_uv - sub_pixel_precision < size_of_int_bits;
        if(!first_test) return;
        if(!divide) {
            const bool second_test= bits_used_area + bits_used_max_uv - sub_pixel_precision +
                    (precision_one_over_area+1) < size_of_big_int_bits;
            if(!second_test) return;
        }
    }
    // once we divide, then one_area occupies at most precision_one_over_area=12 bits, this is
    // important for not overflowing when multiplying it with another integer we know, this is the trick for big integers
#define ceil_fixed(val, bits) ((val)&((1<<bits)-1) ? ((val>>bits)+1) : (val>>bits))
#define floor_fixed(val, bits) ((val)>>bits)
    rect bbox; // bounding box
    rint mask = ~((rint(1)<<sub_pixel_precision)-1);
    bbox.left = floor_fixed(functions::min<rint>(v0_x, v1_x, v2_x)&mask, sub_pixel_precision);
    bbox.top = floor_fixed(functions::min<rint>(v0_y, v1_y, v2_y)&mask, sub_pixel_precision);
    bbox.right = ceil_fixed(functions::max<rint>(v0_x, v1_x, v2_x), sub_pixel_precision);
    bbox.bottom = ceil_fixed(functions::max<rint>(v0_y, v1_y, v2_y), sub_pixel_precision);
    bbox = bbox.intersect(effectiveRect); // raster clipping
#undef ceil_fixed
#undef floor_fixed

    precision bits_distance = 0; // anti-alias pad for distance calculation
    precision bits_distance_complement = 8;
    unsigned int max_distance_scaled_space_anti_alias=0; // max distance to consider in scaled space
    bool aa_all_edges = false;
    if(antialias) {
        unsigned int max_distance_canvas_space_anti_alias=0;
        aa_all_edges = aa_first_edge && aa_second_edge && aa_third_edge;
        bits_distance = 0;
        bits_distance_complement = 8 - bits_distance;
        max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias << P_AA;
    }
    triangles::top_left_t top_left = // fill rules adjustments
            triangles::classifyTopLeftEdges<rint>(false, v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    int bias_w0=top_left.first?0:-1, bias_w1=top_left.second?0:-1, bias_w2=top_left.third?0:-1;
    vec2<rint> p = { bbox.left, bbox.top }; // Barycentric coordinates at minX/minY corner
    vec2<rint> p_fixed = { bbox.left<<sub_pixel_precision, bbox.top<<sub_pixel_precision };
    p_fixed = p_fixed + vec2<rint> {(rint(1)<<(sub_pixel_precision))>>1, (rint(1)<<(sub_pixel_precision))>>1}; // we sample at the center
    // this can produce a 2B+2P bits number if the points form a a perpendicular triangle
    // this is my patent for correct fill rules without wasting bits, amazingly works and accurate,
    // I still need to explain to myself why it works so well :) compress down to (2B+P) after
    rint w0_row = rint((functions::orient2d<int, rint_big>(v0_x,v0_y, v1_x,v1_y, p_fixed.x,p_fixed.y, 0) + bias_w0)>>sub_pixel_precision);
    rint w1_row = rint((functions::orient2d<int, rint_big>(v1_x,v1_y, v2_x,v2_y, p_fixed.x,p_fixed.y, 0) + bias_w1)>>sub_pixel_precision);
    rint w2_row = rint((functions::orient2d<int, rint_big>(v2_x,v2_y, v0_x,v0_y, p_fixed.x,p_fixed.y, 0) + bias_w2)>>sub_pixel_precision);
    rint A01 = (v0_y - v1_y), A12 = (v1_y - v2_y), A20 = (v2_y - v0_y);
    rint B01 = (v1_x - v0_x), B12 = (v2_x - v1_x), B20 = (v0_x - v2_x);
    rint w0_row_h=0, w1_row_h=0, w2_row_h=0;
    rint A01_h=0, B01_h=0, A12_h=0, B12_h=0, A20_h=0, B20_h=0;
    if(antialias) { // lengths of edges, produces a P+1 bits number
        // AA, 2A/L = h, therefore the division produces a P bit number
        unsigned int length_w0 = microgl::math::distance(v0_x, v0_y, v1_x, v1_y);
        unsigned int length_w1 = microgl::math::distance(v1_x, v1_y, v2_x, v2_y);
        unsigned int length_w2 = microgl::math::distance(v0_x, v0_y, v2_x, v2_y);
        A01_h = (((rint_big)(v0_y - v1_y))<<P_AA)/length_w0, B01_h = (((rint_big)(v1_x - v0_x))<<P_AA)/length_w0;
        A12_h = (((rint_big)(v1_y - v2_y))<<P_AA)/length_w1, B12_h = (((rint_big)(v2_x - v1_x))<<P_AA)/length_w1;
        A20_h = (((rint_big)(v2_y - v0_y))<<P_AA)/length_w2, B20_h = (((rint_big)(v0_x - v2_x))<<P_AA)/length_w2;
        w0_row_h = (((rint_big)(w0_row))<<P_AA)/length_w0;
        w1_row_h = (((rint_big)(w1_row))<<P_AA)/length_w1;
        w2_row_h = (((rint_big)(w2_row))<<P_AA)/length_w2;
    }
    const int pitch= width();
    int index = p.y * pitch;
    for (p.y = bbox.top; p.y <= bbox.bottom; p.y++, index+=pitch) {
        rint w0=w0_row, w1=w1_row, w2=w2_row, w0_h=0, w1_h=0, w2_h=0;
        if(antialias) { w0_h=w0_row_h; w1_h=w1_row_h; w2_h=w2_row_h; }
        for (p.x = bbox.left; p.x <= bbox.right; p.x++) {
            bool should_sample=false;
            uint8_t blend=opacity;
            if((w0|w1|w2)>=0) should_sample=true;
            else if(antialias) { // cheap AA based on SDF
                const rint distance = functions::min<rint>((w0_h), (w1_h), (w2_h));
                // delta is at most 16 bits
                rint delta = distance+max_distance_scaled_space_anti_alias;
                bool perform_aa = (delta>=0) && (aa_all_edges || ((distance == (w0_h)) && aa_first_edge) ||
                                               ((distance == (w1_h)) && aa_second_edge) ||
                                               ((distance == (w2_h)) && aa_third_edge));
                if (perform_aa) {
                    should_sample = true; // 16+8=24 bits
                    blend = functions::clamp<rint>((delta<<bits_distance_complement)>>P_AA, 0, 255);
                    blend = (blend*opacity)>>8;
                }
            }
            if(should_sample) {
                rint u_i=0, v_i=0;
                const auto pp = sub_pixel_precision;
                // I compress down the weights to save some bits.
                // bits=(bits_area_used+bits_used_max_uv) - sub_pixel_precision
                rint w0_c=w0>>pp, w1_c=w1>>pp, w2_c=w2>>pp;
                rint u_fixed = (w0_c*rint(u2)) + (w1_c*rint(u0)) + (w2_c*rint(u1));
                rint v_fixed = (w0_c*rint(v2)) + (w1_c*rint(v0)) + (w2_c*rint(v1));
                if(perspective_correct) {
                    rint q_fixed = (w0_c*rint(q2)) +
                                   (w1_c*rint(q0)) + (w2_c*rint(q1));
                    rint q_compressed=q_fixed>>uv_precision; // this would not render with overflow detection
                    if(q_compressed) { // compression has a pitfall of producing zero
                        u_i = rint(u_fixed/q_compressed); v_i = rint(v_fixed/q_compressed);
                    }
                } else {
                    if(divide) { // division is stabler and is un-avoidable most of the time for pure 32 bit mode
                        rint aaa = w0_c+w1_c+w2_c;
                        if(aaa){
                            u_i = (u_fixed)/aaa; v_i = (v_fixed)/aaa;
                        }
                    } else { // we use a temporary 64 bit and one_area to mimic division, this is FASTER even in 32 bits mode.
//                        u_fixed=u_fixed>>sub_pixel_precision;v_fixed=v_fixed>>sub_pixel_precision; // compress the bits, still is fine
                        rint_big u_fixed = rint_big(rint_big(w0)*rint_big(u2) + rint_big(w1)*rint_big(u0)+rint_big(w2)*rint_big(u1) )>>pp;
                        rint_big v_fixed = rint_big(rint_big(w0)*rint_big(v2) + rint_big(w1)*rint_big(v0)+rint_big(w2)*rint_big(v1) )>>pp;
                        u_i = rint_big(rint_big(u_fixed)*rint_big(one_area))>>(LL-pp);
                        v_i = rint_big(rint_big(v_fixed)*rint_big(one_area))>>(LL-pp);
                    }
                }
//                if(should_sample) {
//                    rint u_i=0, v_i=0;
//                    // I compress down the weights to save some bits.
//                    // bits=(bits_area_used+bits_used_max_uv) - sub_pixel_precision
//                    rint u_fixed = (w0*rint(u2) + w1*rint(u0) + w2*rint(u1));
//                    rint v_fixed = (w0*rint(v2) + w1*rint(v0) + w2*rint(v1));
//                    if(perspective_correct) {
//                        rint q_fixed = ((w0*rint(q2))>>0) +
//                                       ((w1*rint(q0))>>0) + ((w2*rint(q1))>>0);
//                        rint q_compressed=q_fixed>>uv_precision; // this would not render with overflow detection
//                        if(q_compressed) { // compression has a pitfall of producing zero
//                            u_i = rint(u_fixed)/(q_compressed); v_i = rint(v_fixed)/(q_compressed);
//                        }
//                    } else {
//                        if(divide) { // division is stabler and is un-avoidable most of the time for pure 32 bit mode
//                            u_i = (u_fixed)/area; v_i = (v_fixed)/area;
//                        } else { // we use a temporary 64 bit and one_area to mimic division, this is FASTER even in 32 bits mode.
//                            u_fixed=u_fixed>>sub_pixel_precision;v_fixed=v_fixed>>sub_pixel_precision; // compress the bits, still is fine
//                            u_i = rint_big(rint_big(u_fixed)*rint_big(one_area))>>(LL-sub_pixel_precision);
//                            v_i = rint_big(rint_big(v_fixed)*rint_big(one_area))>>(LL-sub_pixel_precision);
//                        }
//                    }
                u_i = functions::clamp<rint>(u_i, 0, (rint(1)<<uv_precision));
                v_i = functions::clamp<rint>(v_i, 0, (rint(1)<<uv_precision));
                color_t col_bmp;
                sampler.sample(u_i, v_i, uv_precision, col_bmp);
                blendColor<BlendMode, PorterDuff, S::a>(col_bmp, index + p.x, blend);
            }
            w0+=A01; w1+=A12; w2+=A20;
            if(antialias) { w0_h+=A01_h; w1_h+=A12_h; w2_h+=A20_h; }
        }
        w0_row+=B01; w1_row+=B12; w2_row+=B20;
        if(antialias) { w0_row_h+=B01_h; w1_row_h+=B12_h; w2_row_h+=B20_h; }
    }
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawTriangle(const sampler<S> & sampler,
                                           const number1 &v0_x, const number1 &v0_y, const number2 &u0, const number2 &v0,
                                           const number1 &v1_x, const number1 &v1_y, const number2 &u1, const number2 &v1,
                                           const number1 &v2_x, const number1 &v2_y, const number2 &u2, const number2 &v2,
                                           const opacity_t opacity, bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    const precision prec_pixel = renderingOptions()._2d_raster_bits_sub_pixel,
        prec_uv = renderingOptions()._2d_raster_bits_uv;
#define f_pos(v) microgl::math::to_fixed((v), prec_pixel)
#define f_uv(v) microgl::math::to_fixed((v), prec_uv)
    drawTriangle<BlendMode, PorterDuff, antialias, false, S>(sampler,
            f_pos(v0_x), f_pos(v0_y), f_uv(u0), f_uv(v0), f_uv(0),
            f_pos(v1_x), f_pos(v1_y), f_uv(u1), f_uv(v1), f_uv(0),
            f_pos(v2_x), f_pos(v2_y), f_uv(u2), f_uv(v2), f_uv(0),
            opacity, prec_pixel, prec_uv,
            aa_first_edge, aa_second_edge, aa_third_edge);
#undef f_pos
#undef f_uv
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number, typename depth_buffer_type>
void canvas<BITMAP, options>::drawTriangle(shader<impl, vertex_attr, varying, number> &shader,
                                           int viewport_width, int viewport_height,
                                           vertex_attr v0, vertex_attr v1, vertex_attr v2,
                                           const opacity_t opacity, const triangles::face_culling & culling,
                                           depth_buffer_type *depth_buffer,
                                           const number& depth_range_near, const number& depth_range_far) {
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
        constexpr precision p = 15; // this is not a problem, bary coords of clipping are always [0, 1] with bary.w=1
        const vec4<int> bary_0_fixed= {f(bary_0.x, p), f(bary_0.y, p), f(bary_0.z, p), f(bary_0.w, p)};
        const vec4<int> bary_1_fixed= {f(bary_1.x, p), f(bary_1.y, p), f(bary_1.z, p), f(bary_1.w, p)};
        const vec4<int> bary_2_fixed= {f(bary_2.x, p), f(bary_2.y, p), f(bary_2.z, p), f(bary_2.w, p)};
        varying_v0_clip.interpolate(varying_v0, varying_v1, varying_v2, bary_0_fixed);
        varying_v1_clip.interpolate(varying_v0, varying_v1, varying_v2, bary_1_fixed);
        varying_v2_clip.interpolate(varying_v0, varying_v1, varying_v2, bary_2_fixed);
        drawTriangle_shader_homo_internal<BlendMode, PorterDuff, antialias, perspective_correct, depth_buffer_flag,
        impl, vertex_attr, varying, number>(
                shader, viewport_width, viewport_height,
                p0, p1, p2,
                varying_v0_clip, varying_v1_clip, varying_v2_clip,
                opacity, culling, depth_buffer, depth_range_near, depth_range_far);
    }
#undef f
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number, typename depth_buffer_type>
void canvas<BITMAP, options>::drawTriangle_shader_homo_internal(shader<impl, vertex_attr, varying, number> & $shader,
                                                                int viewport_width, int viewport_height,
                                                                const vec4<number> &p0, const vec4<number> &p1, const vec4<number> &p2,
                                                                varying &varying_v0, varying &varying_v1, varying &varying_v2,
                                                                opacity_t opacity, const triangles::face_culling & culling,
                                                                depth_buffer_type * depth_buffer,
                                                                number depth_range_near, number depth_range_far) {
    /*
     * given triangle coords in a homogeneous coords, a $shader, and corresponding interpolated varying
     * vertex attributes. we pass varying because somewhere in the pipeline we might have clipped things
     * in homogeneous space and therefore had to update/correct the vertex attributes.
     */
    using shader_type = shader<impl, vertex_attr, varying, number>;
    auto & zbuff=*depth_buffer;
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
    const precision sub_pixel_precision = renderingOptions()._3d_raster_bits_sub_pixel;
    const precision w_bits= renderingOptions()._3d_raster_bits_w;
#define f microgl::math::to_fixed
    varying interpolated_varying;
    // perspective divide by w -> NDC space
    if(p0.w==0 || p1.w==0 || p2.w==0) return;
    const auto v0_ndc = p0/p0.w; const auto one_over_w0=number(1)/(p0.w);
    const auto v1_ndc = p1/p1.w; const auto one_over_w1=number(1)/(p1.w);
    const auto v2_ndc = p2/p2.w; const auto one_over_w2=number(1)/(p2.w);
    // viewport transform: NDC space -> window/viewport/raster space
    const number w= viewport_width, h= viewport_height;
    const number drn=microgl::functions::clamp<number>(depth_range_near, 0, 1);
    const number drf=microgl::functions::clamp<number>(depth_range_far, 0, 1);
    const number range =(drf-drn)/number(2), one = number(1), two=number(2);
    vec3<number> v0_viewport = {((v0_ndc.x + one)*w)/two, h - ((v0_ndc.y + one)*h)/two, drn+(v0_ndc.z+one)*range};
    vec3<number> v1_viewport = {((v1_ndc.x + one)*w)/two, h - ((v1_ndc.y + one)*h)/two, drn+(v1_ndc.z+one)*range};
    vec3<number> v2_viewport = {((v2_ndc.x + one)*w)/two, h - ((v2_ndc.y + one)*h)/two, drn+(v2_ndc.z+one)*range};
    // collect values for interpolation as fixed point integers
    int v0_x= f(v0_viewport.x, sub_pixel_precision), v0_y= f(v0_viewport.y, sub_pixel_precision);
    int v1_x= f(v1_viewport.x, sub_pixel_precision), v1_y= f(v1_viewport.y, sub_pixel_precision);
    int v2_x= f(v2_viewport.x, sub_pixel_precision), v2_y= f(v2_viewport.y, sub_pixel_precision);
    rint area = functions::orient2d<rint, rint_big>(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, sub_pixel_precision);
    rint one_over_w0_fixed= f(one_over_w0, w_bits), one_over_w1_fixed= f(one_over_w1, w_bits),
                                one_over_w2_fixed= f(one_over_w2, w_bits);
    /// overflow detection
    if(options_avoid_overflow()) { // compile time flag
        precision size_of_int_bits = sizeof(rint)<<3, size_of_big_int_bits = sizeof(rint_big)<<3;
        auto bits_used_max_area=microgl::functions::used_integer_bits(area);
        auto bits_used_max_w=microgl::functions::used_integer_bits(microgl::functions::abs_max(
                {one_over_w0_fixed, one_over_w1_fixed, one_over_w2_fixed}));
        const bool first_test = bits_used_max_area + bits_used_max_w < size_of_int_bits;
        if(!first_test) return;
    }
    rint v0_z= rint(v0_viewport.z*zbuff.maxValue()), v1_z= rint(v1_viewport.z*zbuff.maxValue()),
    v2_z=rint(v2_viewport.z*zbuff.maxValue());
    // infer back-face culling
    const bool ccw = area<0;
    if(area==0) return; // discard degenerate triangles
    if(ccw && culling==triangles::face_culling::ccw) return;
    if(!ccw && culling==triangles::face_culling::cw) return;
    if(ccw) { // convert CCW to CW triangle
        functions::swap(v1_x, v2_x); functions::swap(v1_y, v2_y);
        area = -area;
    } else { // flip vertically
        functions::swap(varying_v1, varying_v2);
        functions::swap(one_over_w1_fixed, one_over_w2_fixed); functions::swap(v1_z, v2_z);
    }
    // rotate to match edges
    functions::swap(varying_v0, varying_v1);
    functions::swap(one_over_w0_fixed, one_over_w1_fixed); functions::swap(v0_z, v1_z);
    // bounding box in raster space
#define ceil_fixed(val, bits) ((val)&((1<<bits)-1) ? ((val>>bits)+1) : (val>>bits))
#define floor_fixed(val, bits) ((val)>>bits)
    rect bbox;
    int mask = ~((int(1)<<sub_pixel_precision)-1);
    bbox.left = floor_fixed(functions::min<int>(v0_x, v1_x, v2_x)&mask, sub_pixel_precision);
    bbox.top = floor_fixed(functions::min<int>(v0_y, v1_y, v2_y)&mask, sub_pixel_precision);
    bbox.right = ceil_fixed(functions::max<int>(v0_x, v1_x, v2_x), sub_pixel_precision);
    bbox.bottom = ceil_fixed(functions::max<int>(v0_y, v1_y, v2_y), sub_pixel_precision);
    bbox = bbox.intersect(effectiveRect);
    if(bbox.empty()) return;
#undef ceil_fixed
#undef floor_fixed
    // fill rules configurations
    triangles::top_left_t top_left =
            triangles::classifyTopLeftEdges(false,
                                            v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    int bias_w0=top_left.first?0:-1, bias_w1=top_left.second?0:-1, bias_w2=top_left.third?0:-1;
    // Barycentric coordinates at minX/minY corner
    vec2<int> p = { bbox.left, bbox.top };
    vec2<int> p_fixed = { bbox.left<<sub_pixel_precision, bbox.top<<sub_pixel_precision };
    l64 half= (int(1)<<(sub_pixel_precision))>>1; // todo: remove l64
    p_fixed = p_fixed + vec2<int> {half, half}; // we sample at the center
    // this can produce a 2P bits number if the points form a a perpendicular triangle
    // this is my patent for correct fill rules without wasting bits, amazingly works and accurate,
    // I still need to explain to myself why it works so well :)
    rint b0_row = (functions::orient2d<int, rint_big>(v0_x, v0_y, v1_x, v1_y, p_fixed.x, p_fixed.y, 0) +
                   bias_w0)>>sub_pixel_precision;
    rint b1_row = (functions::orient2d<int, rint_big>(v1_x, v1_y, v2_x, v2_y, p_fixed.x, p_fixed.y, 0) +
                   bias_w1)>>sub_pixel_precision;
    rint b2_row = (functions::orient2d<int, rint_big>(v2_x, v2_y, v0_x, v0_y, p_fixed.x, p_fixed.y, 0) +
                   bias_w2)>>sub_pixel_precision;
    // Triangle setup, this needs at least (P+1) bits, since the delta is always <= length
    rint A01 = v0_y-v1_y, A12 = v1_y-v2_y, A20 = v2_y-v0_y;
    rint B01 = v1_x-v0_x, B12 = v2_x-v1_x, B20 = v0_x-v2_x;
    const int pitch= width(); int index = p.y * pitch;
    for (p.y = bbox.top; p.y <= bbox.bottom; p.y++, index+=pitch) {
        rint b0 = b0_row, b1 = b1_row, b2 = b2_row;
        for (p.x = bbox.left; p.x<=bbox.right; p.x++) {
            // closure test with full sub pixel precision
            const bool in_closure= (b0 | b1 | b2) >= 0;
            bool should_sample= in_closure;
            auto opacity_sample = opacity;
//            auto bary = vec4<rint>{b0, b1, b2, area};
            rint b0_c = b0>>sub_pixel_precision, b1_c = b1>>sub_pixel_precision, b2_c = b2>>sub_pixel_precision;
            rint area_c = b0_c + b1_c + b2_c;
            auto bary = vec4<rint>{b0_c, b1_c, b2_c, area_c};
            if(in_closure && perspective_correct) { // compute perspective-correct and transform to sub-pixel-space
                bary.x= (b0_c * one_over_w0_fixed) >> w_bits;
                bary.y= (b1_c * one_over_w1_fixed) >> w_bits;
                bary.z= (b2_c * one_over_w2_fixed) >> w_bits;
                bary.w=bary.x+bary.y+bary.z;
                if(bary.w==0) bary={1,1,1,3};
            }
            if(depth_buffer_flag && should_sample) {
                using z_type=typename depth_buffer_type::type;
                constexpr bool use_fpu=microgl::traits::is_float_point<number>(); // compile-time flag
                rint denom= rint(v0_z) * b0_c + rint(v1_z) * b1_c + rint(v2_z) * b2_c;
                z_type z=use_fpu ? z_type(number(denom)/area_c) : denom/area_c;
                const int z_index = index-_window.index_correction+p.x;
                if((z>zbuff[z_index])) should_sample=false;
                else zbuff[z_index]=z;
            }
            if(should_sample) {
                // cast to user's number types vec4<number> casted_bary= bary;, I decided to stick with l64
                // because other wise this would have wasted bits for Q types although it would have been more elegant.
                interpolated_varying.interpolate(varying_v0, varying_v1, varying_v2, bary);
                auto color = $shader.fragment(interpolated_varying);
                blendColor<BlendMode, PorterDuff, shader_type::rgba::a>(color, index + p.x, opacity_sample);
            }
            b0+=A01; b1+=A12; b2+=A20;
        }
        b0_row+=B01; b1_row+=B12; b2_row+=B20;
    }
#undef f
}

template<typename BITMAP, uint8_t options>
template <typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawMask(const masks::chrome_mode &mode,
                                       const S & sampler,
                                       const number1 left, const number1 top,
                                       const number1 right, const number1 bottom,
                                       const number2 u0, const number2 v0,
                                       const number2 u1, const number2 v1,
                                       const opacity_t opacity) {
    precision p_sub = renderingOptions()._2d_raster_bits_sub_pixel, p_uv = renderingOptions()._2d_raster_bits_uv;
    drawMask<S>(mode, sampler,
            microgl::math::to_fixed(left, p_sub), microgl::math::to_fixed(top, p_sub),
            microgl::math::to_fixed(right, p_sub), microgl::math::to_fixed(bottom, p_sub),
            microgl::math::to_fixed(u0, p_uv), microgl::math::to_fixed(v0, p_uv),
            microgl::math::to_fixed(u1, p_uv), microgl::math::to_fixed(v1, p_uv),
            p_sub, p_uv, opacity
    );
}

template<typename BITMAP, uint8_t options>
template <typename S>
void canvas<BITMAP, options>::drawMask(const masks::chrome_mode &mode,
                                       const S & sampler,
                                       const int left, const int top,
                                       const int right, const int bottom,
                                       int u0, int v0,
                                       int u1, int v1,
                                       const precision sub_pixel_precision,
                                       const precision uv_precision,
                                       const opacity_t opacity) {
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
#define ceil_fixed(val, bits) ((val)&((1<<bits)-1) ? ((val>>bits)+1) : (val>>bits))
#define floor_fixed(val, bits) (val>>bits)
    color_t col_bmp{};
    const precision p= sub_pixel_precision;
    const rect bbox_r = {floor_fixed(left, p), floor_fixed(top, p),
                         ceil_fixed(right, p)-1, ceil_fixed(bottom, p)-1};
    const rect bbox_r_c = bbox_r.intersect(effectiveRect);
    if(bbox_r_c.empty()) return;
#undef ceil_fixed
#undef floor_fixed
    // calculate original uv deltas, this way we can always accurately predict blocks
    // calculate uvs with original unclipped deltas, this way we can always accurately predict blocks
    const auto bits_du=microgl::functions::used_integer_bits(u1-u0);
    const auto bits_dv=microgl::functions::used_integer_bits(v1-v0);
    const auto bits_width=microgl::functions::used_integer_bits(bbox_r.width());
    const auto bits_height=microgl::functions::used_integer_bits(bbox_r.height());
    // boost is done against compressed sub pixel coords, so we really cannot overflow, we leave
    // (31-14=17) bits for coords without sub-pixel
    const precision boost_bits=14, boost_width= bits_width + boost_bits, boost_height= bits_height + boost_bits;
    precision boost_u=0, boost_v=0;
    if(boost_width > bits_du) boost_u= boost_width - bits_du;
    if(boost_height > bits_dv) boost_v= boost_height - bits_dv;
    u0=u0<<boost_u;v0=v0<<boost_v;u1=u1<<boost_u;v1=v1<<boost_v; // this is (coords-sub_pixel bits)+(boost_bits=14) bits
    const int du= (u1-u0)/bbox_r.width(); // this occupies (boost_bits=14) bits
    const int dv = (v1-v0)/bbox_r.height(); // this occupies (boost_bits=14) bits
    const int dx= bbox_r_c.left-bbox_r.left, dy= bbox_r_c.top-bbox_r.top;
    const int u_start= u0+(du>>1)+dx*du, pitch= width();
    int index= bbox_r_c.top * pitch;
    constexpr bits alpha_bits = pixel_coder::rgba::a ? pixel_coder::rgba::a : 8;
    constexpr channel max_alpha_value = (uint16_t(1)<<alpha_bits) - 1;
    for (int y=bbox_r_c.top, v=v0+(du>>1)+dy*dv; y<=bbox_r_c.bottom; y++, v+=dv, index+=pitch) {
        for (int x=bbox_r_c.left, u=u_start; x<=bbox_r_c.right; x++, u+=du) {
            sampler.sample(u>>boost_u, v>>boost_v, uv_precision, col_bmp);
            channel a=0;
            switch (mode) {
                case masks::chrome_mode::red_channel:
                    a = convert_channel_correct<S::rgba::r, alpha_bits>(col_bmp.r);
                    break;
                case masks::chrome_mode::red_channel_inverted:
                    a = max_alpha_value - convert_channel_correct<S::rgba::r, alpha_bits>(col_bmp.r);
                    break;
                case masks::chrome_mode::alpha_channel:
                    a = convert_channel_correct<S::rgba::a, alpha_bits>(col_bmp.a);
                    break;
                case masks::chrome_mode::alpha_channel_inverted:
                    a = max_alpha_value - convert_channel_correct<S::rgba::a, alpha_bits>(col_bmp.a);
                    break;
                case masks::chrome_mode::green_channel:
                    a = convert_channel_correct<S::rgba::g, alpha_bits>(col_bmp.g);
                    break;
                case masks::chrome_mode::green_channel_inverted:
                    a = max_alpha_value - convert_channel_correct<S::rgba::g, alpha_bits>(col_bmp.g);
                    break;
                case masks::chrome_mode::blue_channel:
                    a = convert_channel_correct<S::rgba::b, alpha_bits>(col_bmp.b);
                    break;
                case masks::chrome_mode::blue_channel_inverted:
                    a = max_alpha_value - convert_channel_correct<S::rgba::b, alpha_bits>(col_bmp.b);
                    break;
            }
            col_bmp.r=0, col_bmp.g=0, col_bmp.b=0, col_bmp.a=a,
            // re-encode for a different canvas
            blendColor<blendmode::Normal, porterduff::DestinationIn<true>, alpha_bits>(col_bmp, index + x, opacity);
        }
    }
}

template<typename BITMAP, uint8_t options>
template <microgl::polygons::hints hint, typename BlendMode, typename PorterDuff, bool antialias, bool debug, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawPolygon(const sampler<S> &sampler,
                                          const matrix_3x3<number1> &transform,
                                          const vec2<number1> *points,
                                          index size, opacity_t opacity,
                                          const number2 u0, const number2 v0,
                                          const number2 u1, const number2 v1) {
    indices type;
    dynamic_array<index> indices;
    dynamic_array<boundary_info> boundary_buffer;
    dynamic_array<boundary_info> *boundary_buffer_ptr=antialias? &boundary_buffer: nullptr;
    switch (hint) {
        case hints::CONCAVE:
        case hints::SIMPLE:
        {
            using ect=microgl::tessellation::ear_clipping_triangulation<number1>;
            ect::compute(points, size, indices, boundary_buffer_ptr, type);
            break;
        }
        case hints::X_MONOTONE:
        case hints::Y_MONOTONE:
        {
            using mpt=microgl::tessellation::monotone_polygon_triangulation<number1>;
            typename mpt::monotone_axis axis=hint==hints::X_MONOTONE ? mpt::monotone_axis::x_monotone :
                                    mpt::monotone_axis::y_monotone;
            mpt::compute(points, size, axis, indices, boundary_buffer_ptr, type);
            break;
        }
        case hints::CONVEX:
        {
            using fan=microgl::tessellation::fan_triangulation<number1>;
            fan::compute(points, size, indices, boundary_buffer_ptr, type);
            break;
        }
        case hints::NON_SIMPLE:
        case hints::SELF_INTERSECTING:
        case hints::COMPLEX:
        case hints::MULTIPLE_POLYGONS:
        {
            tessellation::path<number1> path{};
            path.addPoly(points, size);
            drawPathFill<BlendMode, PorterDuff, antialias, debug, number1, number2, S>(
                    sampler, transform, path, tessellation::fill_rule::non_zero,
                    tessellation::tess_quality::better, opacity, u0, v0, u1, v1);
            return;
        }
        default:
            return;
    }
    const vec2<number2> * uvs= nullptr;//uv_map<number1, number2>::compute(points, size, u0, v0, u1, v1);
    drawTriangles<BlendMode, PorterDuff, antialias, number1, number2, S>(
            sampler, transform,
            points,
            uvs,
            indices.data(),
            boundary_buffer.data(),
            indices.size(),
            type,
            opacity,
            u0, v0, u1, v1);

    if(debug)
        drawTrianglesWireframe({0,0,0,255},
                transform, points,
                indices.data(), indices.size(),
                type, 255);
}

template<typename BITMAP, uint8_t options>
template <typename BlendMode, typename PorterDuff, bool antialias, bool debug, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawPathStroke(const sampler<S> &sampler,
                                             const matrix_3x3<number1> &transform,
                                             tessellation::path<number1> & path,
                                             const number1 & stroke_width,
                                             const tessellation::stroke_cap &cap,
                                             const tessellation::stroke_line_join &line_join,
                                             const int miter_limit,
                                             const std::initializer_list<int> & stroke_dash_array,
                                             int stroke_dash_offset,
                                             opacity_t opacity,
                                             const number2 u0, const number2 v0,
                                             const number2 u1, const number2 v1) {
    const auto & buffers= path.tessellateStroke(stroke_width, cap, line_join, miter_limit, stroke_dash_array, stroke_dash_offset);
    drawTriangles<BlendMode, PorterDuff, antialias, number1, number2, S>(
            sampler, transform,
            buffers.output_vertices.data(),
            static_cast<vec2<number2> *>(nullptr),
            buffers.output_indices.data(),
            buffers.output_boundary.data(),
            buffers.output_indices.size(),
            buffers.output_indices_type,
            opacity,
            u0, v0, u1, v1);
    if(debug)
        drawTrianglesWireframe({0,0,0,255}, transform,
                               buffers.output_vertices.data(),
                               buffers.output_indices.data(),
                               buffers.output_indices.size(),
                               buffers.output_indices_type,
                               255);
}

template<typename BITMAP, uint8_t options>
template <typename BlendMode, typename PorterDuff, bool antialias, bool debug, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawPathFill(const sampler<S> &sampler,
                                           const matrix_3x3<number1> &transform,
                                           tessellation::path<number1> & path,
                                           const tessellation::fill_rule &rule,
                                           const tessellation::tess_quality &quality,
                                           opacity_t opacity,
                                           const number2 u0, const number2 v0,
                                           const number2 u1, const number2 v1) {
    const auto & buffers= path.tessellateFill(rule, quality);
    if(buffers.output_vertices.size()==0) return;
    drawTriangles<BlendMode, PorterDuff, antialias, number1, number2, S>(
            sampler, transform,
            buffers.output_vertices.data(),
            static_cast<vec2<number2> *>(nullptr),
            buffers.output_indices.data(),
            buffers.output_boundary.data(),
            buffers.output_indices.size(),
            buffers.output_indices_type,
            opacity,
            u0, v0, u1, v1);
    if(debug) {
        drawTrianglesWireframe({0,0,0,255}, transform,
                               buffers.output_vertices.data(),
                               buffers.output_indices.data(),
                               buffers.output_indices.size(),
                               buffers.output_indices_type,
                               40);
        for (index ix = 0; ix < buffers.DEBUG_output_trapezes.size(); ix+=4) {
            drawWuLinePath<number1>({0,0,0,255}, &buffers.DEBUG_output_trapezes[ix], 4, true);
        }
    }
}

template<typename BITMAP, uint8_t options>
template<typename number>
void canvas<BITMAP, options>::drawWuLine(const color_t &color,
                                         const number &x0, const number &y0,
                                         const number &x1, const number &y1,
                                         const opacity_t opacity) {
    using clipper = microgl::clipping::cohen_sutherland_clipper<number>;
    auto draw_rect = calculateEffectiveDrawRect();
    auto clip =  clipper::compute(x0, y0, x1, y1, draw_rect.left, draw_rect.top,
                                  draw_rect.right, draw_rect.bottom);
    if(!clip.inside) return;
    precision p = 8;
    int x0_ = microgl::math::to_fixed(clip.x0, p);
    int y0_ = microgl::math::to_fixed(clip.y0, p);
    int x1_ = microgl::math::to_fixed(clip.x1, p);
    int y1_ = microgl::math::to_fixed(clip.y1, p);
    drawWuLine(color, x0_, y0_, x1_, y1_, p, opacity);
}

template<typename BITMAP, uint8_t options>
void canvas<BITMAP, options>::drawWuLine(const color_t &color,
                                         const int x0, const int y0,
                                         const int x1, const int y1,
                                         precision bits, const opacity_t opacity) {
    constexpr uint8_t a_bits = hasNativeAlphaChannel() ? pixel_coder::rgba::a : 8;
    // we assume that the line is in the closure (interior+boundary) of the canvas window
    int X0 = x0, Y0 = y0, X1 = x1, Y1=y1;
    color_t color_input=color;
    unsigned int IntensityBits = 8;
    unsigned int NumLevels = 1u << IntensityBits;
    unsigned int maxIntensity = NumLevels - 1;
    uint32_t IntensityShift, ErrorAdj, ErrorAcc;
    unsigned int ErrorAccTemp, Weighting, WeightingComplementMask;
    int DeltaX, DeltaY, Temp, XDir;//, YDir;
    int one = int(1)<<bits;
    unsigned int round = 0;//one>>1;// -1;
    // Make sure the line runs top to bottom
    if (Y0 > Y1) { Temp = Y0; Y0 = Y1; Y1 = Temp; Temp = X0; X0 = X1; X1 = Temp; }
    // Draw the initial pixel, which is always exactly intersected by the line and so needs no weighting
    blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input,
                                                              (X0+round)>>bits, (Y0+round)>>bits, opacity);
    if ((DeltaX = X1 - X0) >= 0) {
        XDir = int(1)<<bits;
    } else {
        XDir = -(int(1)<<bits);
        DeltaX = -DeltaX; // make DeltaX positive
    }
    DeltaY = Y1 - Y0;
    // Special-cases
    if ((Y1 - Y0) == 0) { // Horizontal line
        while ((DeltaX-=one) > 0) {
            X0 += XDir;
            blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, X0>>bits, Y0>>bits, opacity);
        }
        return;
    }
    if (DeltaX == 0) { // Vertical line
        do {
            Y0+=one;
            blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, X0>>bits, Y0>>bits, opacity);
        } while ((DeltaY-=one) > 0);
        return;
    }
    if (DeltaX == DeltaY) { // Diagonal line
        do {
            X0 += XDir;
            Y0+=one;
            blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, X0>>bits, Y0>>bits, opacity);
        } while ((DeltaY-=one) > 0);
        return;
    }

    ErrorAcc = 0; // initialize the line error accumulator to 0
    // # of bits by which to shift ErrorAcc to get intensity level
    IntensityShift = 32 - IntensityBits;
    // Mask used to flip all bits in an intensity weighting, producing the result (1 - intensity weighting)
    // The IntensityBits most significant bits of ErrorAcc give us the
    // intensity weighting for this pixel, and the complement of the
    // weighting for the paired pixel
    WeightingComplementMask = maxIntensity;

    if (DeltaY > DeltaX) {
        // Y-major line; calculate 16-bit fixed-point fractional part of a
        // pixel that X advances each time Y advances 1 pixel, truncating the
        // result so that we won't overrun the endpoint along the X axis
        ErrorAdj = ((unsigned long long) DeltaX << 32) / (unsigned long long) DeltaY;
        while ((DeltaY-=one) > 0) { // Draw all pixels other than the first and last
            ErrorAccTemp = ErrorAcc; // remember current accumulated error
            ErrorAcc += ErrorAdj; // calculate error for next pixel
            if (ErrorAcc <= ErrorAccTemp) X0 += XDir; // The error accumulator turned over, so advance the X coord
            Y0+=one;
            Weighting = ErrorAcc >> IntensityShift;
            unsigned int mix = (Weighting ^ WeightingComplementMask); // complement of Weighting
            blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, X0>>bits, Y0>>bits, (mix*opacity*257)>>16);
            blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, (X0 + XDir)>>bits, Y0>>bits, (Weighting*opacity*257)>>16);
        }
        // Draw the final pixel, which is always exactly intersected by the line
        blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, (X1+round)>>bits, (Y1+round)>>bits, opacity);
        return;
    }
    // It's an X-major line;
    ErrorAdj = ((unsigned long long) DeltaY << 32) / (unsigned long long) DeltaX;
    // Draw all pixels other than the first and last
    while ((DeltaX-=one) > 0) {
        ErrorAccTemp = ErrorAcc; // remember currrent accumulated error
        ErrorAcc += ErrorAdj; // calculate error for next pixel
        if (ErrorAcc <= ErrorAccTemp) Y0+=one;
        X0 += XDir; // X-major, so always advance X
        Weighting = (ErrorAcc >> IntensityShift);
        unsigned int mix = (Weighting ^ WeightingComplementMask);
        unsigned int mix_complement = maxIntensity - mix; // this equals Weighting
        blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, X0>>bits, Y0>>bits, (mix*opacity*257)>>16);
        blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, X0>>bits, (Y0 + one)>>bits, (Weighting*opacity*257)>>16);
    }
    // Draw the final pixel, which is always exactly intersected by the line and so needs no weighting
    blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_bits>(color_input, (X1+round)>>bits, (Y1+round)>>bits, opacity);
}

template<typename BITMAP, uint8_t options>
template <typename number>
void
canvas<BITMAP, options>::drawWuLinePath(const color_t &color,
                                        const vec2<number> *points,
                                        unsigned int size,
                                        bool closed_path) {
    index jx = 0;
    for (jx = 0; jx < size; jx++)
        if(jx) drawWuLine(color, points[jx - 1].x, points[jx - 1].y, points[jx].x,
                points[jx].y);
    if(closed_path) drawWuLine(color, points[0].x, points[0].y, points[jx - 1].x,
            points[jx - 1].y);
}

template<typename BITMAP, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, bool debug, typename number1, typename number2, typename S>
void canvas<BITMAP, options>::drawBezierPatch(const sampler<S> & sampler,
                                              const matrix_3x3<number1> &transform,
                                              const vec3<number1> *mesh,
                                              const unsigned uOrder, const unsigned vOrder,
                                              const unsigned uSamples, const unsigned vSamples,
                                              const number2 u0, const number2 v0,
                                              const number2 u1, const number2 v1,
                                              const opacity_t opacity) {
    using tess= microgl::tessellation::bezier_patch_tesselator<number1, number2>;
    using vertex=vec2<number1>;
    dynamic_array<number1> v_a; // vertices attributes
    dynamic_array<index> indices;
    microgl::triangles::indices indices_type;
    tess::compute(mesh, uOrder, vOrder, uSamples, vSamples, v_a,
                  indices, indices_type, u0, v0, u1, v1);
    const index size = indices.size();
    const index window_size=5;
    const index I_X=0, I_Y=1, I_Z=2, I_U=3, I_V=4;
    if(size==0) return;
#define IND(a) indices[(a)]
    bool even = true;
    for (index ix = 0; ix < size-2; ++ix, even=!even) { // we alternate order inorder to preserve CCW or CW,
        index first_index   = (even ? IND(ix + 0) : IND(ix + 2))*window_size;
        index second_index  = (even ? IND(ix + 1) : IND(ix + 1))*window_size;
        index third_index   = (even ? IND(ix + 2) : IND(ix + 0))*window_size;
        vertex p1=vertex{v_a[first_index + I_X], v_a[first_index + I_Y]};
        vertex p2=vertex{v_a[second_index + I_X], v_a[second_index + I_Y]};
        vertex p3=vertex{v_a[third_index + I_X], v_a[third_index + I_Y]};
        p1= transform * p1;p2= transform * p2;p3= transform * p3;
        drawTriangle<BlendMode, PorterDuff, antialias>(sampler,
               p1.x, p1.y, v_a[first_index + I_U],  v_a[first_index + I_V],
               p2.x, p2.y, v_a[second_index + I_U], v_a[second_index + I_V],
               p3.x, p3.y, v_a[third_index + I_U],  v_a[third_index + I_V], opacity); //even = !even;
        if(debug)
            drawTriangleWireframe<number1>(color_t{0,0,0,255},
                                  {v_a[first_index + I_X], v_a[first_index + I_Y]},
                                  {v_a[second_index + I_X], v_a[second_index + I_Y]},
                                  {v_a[third_index + I_X], v_a[third_index + I_Y]});
    }
#undef IND
}

template<typename BITMAP, uint8_t options>
void canvas<BITMAP, options>::fxaa(int left, int top, int right, int bottom) {
    // taken from_sampler opengl cookbook
//    left=160;top=160;right=left+300;bottom=top+300;
//return;
    // this is the config in an optimized manner
    const l64 FXAA_SPAN_PIXELS_MAX = 8; // max pixels span
    const l64 FXAA_REDUCE_MUL_BITS = 3; // to be used as 1/2^3
    const l64 FXAA_REDUCE_MIN_BITS = 4; // to be used as 1/2^7
    const l64 LUMA_THRESHOLD_BITS = 5; // to be used as 1/2^5

    const char p = 10;
    const char t = p - coder().g();
    const l64 ONE = l64(1) << p;
    const l64 FXAA_REDUCE_MIN = ONE >> FXAA_REDUCE_MIN_BITS;
    const l64 FXAA_SPAN_MAX = FXAA_SPAN_PIXELS_MAX << p;
    const int pitch = width();
    for (int yy = top, index = top * pitch; yy < bottom; ++yy, index += pitch) {
        for (int xx = left; xx < right; ++xx) {
            color_t nw, ne, sw, se, m, rgb_1, rgb_2, rgb_3, rgb_4, rgb_A, rgb_B;
            this->_bitmap_canvas->decode(index + xx, m);
            this->_bitmap_canvas->decode(index + xx - pitch - 1, nw);
            this->_bitmap_canvas->decode(index + xx - pitch + 1, ne);
            this->_bitmap_canvas->decode(index + xx + pitch - 1, sw);
            this->_bitmap_canvas->decode(index + xx + pitch + 1, se);
            // convert lumas to p bits space
            l64 luma_M = l64(m.g) << t, luma_NW = l64(nw.g) << t, luma_NE = l64(ne.g) << t, luma_SW =
                    l64(sw.g) << t, luma_SE = l64(se.g) << t;
            l64 luma_min = microgl::functions::min(luma_M, luma_NW, luma_NE, luma_SW, luma_SE);
            l64 luma_max = microgl::functions::max(luma_M, luma_NW, luma_NE, luma_SW, luma_SE);
            // If contrast is lower than a maximum threshold ...
            if (luma_max - luma_min <= (luma_max >> LUMA_THRESHOLD_BITS))
                continue;
            l64 dirSwMinusNe = luma_SW - luma_NE;
            l64 dirSeMinusNw = luma_SE - luma_NW;
            l64 dir_x = dirSwMinusNe + dirSeMinusNw;
            l64 dir_y = dirSwMinusNe - dirSeMinusNw;
            if (dir_x == 0 && dir_y == 0) continue;

            l64 dirReduce = microgl::functions::max(
                    ((luma_NW + luma_NE + luma_SW + luma_SE) >> (FXAA_REDUCE_MUL_BITS + 2)), // +2 for average
                    FXAA_REDUCE_MIN);
            l64 rcpDirMin = microgl::functions::min(microgl::math::abs(dir_x), microgl::math::abs(dir_y)) + dirReduce;
            dir_x = (dir_x << p) / rcpDirMin, dir_y = (dir_y << p) / rcpDirMin;
            dir_x = microgl::functions::clamp(dir_x, -FXAA_SPAN_MAX, FXAA_SPAN_MAX);
            dir_y = microgl::functions::clamp(dir_y, -FXAA_SPAN_MAX, FXAA_SPAN_MAX);
            l64 dir_x_temp = (dir_x / 4) >> p, dir_y_temp = (dir_y / 4) >> p;
            this->_bitmap_canvas->decode(xx - dir_x_temp, yy - dir_y_temp, rgb_1);
            this->_bitmap_canvas->decode(xx + dir_x_temp, yy + dir_y_temp, rgb_2);
            rgb_A.r = (int(rgb_1.r) + int(rgb_2.r)) >> 1, rgb_A.g = (int(rgb_1.g) + int(rgb_2.g)) >> 1, rgb_A.b =
                    (int(rgb_1.b) + int(rgb_2.b)) >> 1;
            dir_x_temp = (dir_x >> 1) >> p, dir_y_temp = (dir_y >> 1) >> p;
            this->_bitmap_canvas->decode(xx - dir_x_temp, yy - dir_y_temp, rgb_3);
            this->_bitmap_canvas->decode(xx + dir_x_temp, yy + dir_y_temp, rgb_4);
            rgb_B.r = int(rgb_A.r >> 1) + ((int(rgb_3.r) + int(rgb_4.r)) >> 2); // compute the average of 4 pixels
            rgb_B.g = int(rgb_A.g >> 1) + ((int(rgb_3.g) + int(rgb_4.g)) >> 2);
            rgb_B.b = int(rgb_A.b >> 1) + ((int(rgb_3.b) + int(rgb_4.b)) >> 2);
            l64 luma_rgb_B = l64(rgb_B.g) << t;
            color_t *selected_color;
            if (luma_rgb_B < luma_min || luma_rgb_B > luma_max) selected_color = &rgb_A;
            else selected_color = &rgb_B;
//            selected_color=&rgb_B;
            selected_color->a = m.a; // restore middle alpha
            pixel output;
            color_t black{0, 0, 0};
//            selected_color=&black;
            coder().encode(*selected_color, output);
            drawPixel(output, index + xx);
        }
    }
}

#include <microgl/samplers/texture.h>
template<typename BITMAP, uint8_t options>
template<bool tint, typename BITMAP_FONT_TYPE>
void canvas<BITMAP, options>::drawText(const char * text, microgl::text::bitmap_font<BITMAP_FONT_TYPE> &font,
                                       const color_t & color, microgl::text::text_format & format,
                                       int left, int top, int right, int bottom, bool frame,
                                       opacity_t opacity) {
    rect old=clipRect(); updateClipRect(left, top, right, bottom);
//    microgl::sampling::texture<BITMAP_FONT_TYPE, sampling::texture_filter::NearestNeighboor> texture{font._bitmap};
//    drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false>(texture,0,0, font._bitmap->width(), font._bitmap->height());return;
    unsigned int text_size=0;
    { const char * iter=text; while(*iter++!= '\0' && ++text_size); }
    microgl::text::char_location loc_buffer[text_size];
    const auto result=font.layout_text(text, text_size, right-left, bottom-top, format, loc_buffer);
    unsigned count= result.end_index;
    const int s=result.scale, PP=result.precision;
    const bool has_scaled=s!=1<<PP;
    if(has_scaled){ // we use the sampler for scaled
        using tex = microgl::sampling::texture<BITMAP_FONT_TYPE, sampling::texture_filter::NearestNeighboor, tint>;
         tex texture{font._bitmap, color};
        const int UVP=renderingOptions()._2d_raster_bits_uv;
        int u0, v0, u1, v1;
        for (unsigned ix = 0; ix < count; ++ix) {
            const auto & l= result.locations[ix];
            u0=(l.character->x<<UVP)/font._bitmap->width();
            u1=((l.character->x+l.character->width)<<UVP)/font._bitmap->width();
            v0=((font._bitmap->height()-l.character->y)<<UVP)/font._bitmap->height();
            v1=((font._bitmap->height()-l.character->y-l.character->height)<<UVP)/font._bitmap->height();
            v0=((l.character->y)<<UVP)/font._bitmap->height();
            v1=((l.character->y+l.character->height)<<UVP)/font._bitmap->height();
            int ll= l.x; ll+=left<<PP; int tt= l.y; tt+=top<<PP;
            int rr= ll + ((l.character->width*s)); int bb= tt + ((l.character->height*s));
            drawRect<blendmode::Normal, porterduff::FastSourceOverOnOpaque, false, decltype(texture)>(
                    texture, ll, tt, rr, bb, u0, v0, u1, v1, PP, UVP, opacity
            );
        }
    }
    else { // the sampler above gives amazing results for unscaled graphics, but I  to go for the most accurate version
        constexpr uint8_t r_ = BITMAP_FONT_TYPE::r;
        constexpr uint8_t g_ = BITMAP_FONT_TYPE::g;
        constexpr uint8_t b_ = BITMAP_FONT_TYPE::b;
        constexpr uint8_t a_ = BITMAP_FONT_TYPE::a;
        for (unsigned ix = 0; ix < count; ++ix) {
            const auto & l= result.locations[ix];
            const auto & c= *l.character;
            int ll= l.x>>PP; ll+= left; int tt= l.y>>PP; tt+=top;
            int rr= ll+c.width; int bb= tt+c.height;
            rect box{ll, tt, rr, bb};
            rect draw_rect=calculateEffectiveDrawRect();
            auto b_r=box.intersect(draw_rect);
            color_t font_col;
            for (int y = b_r.top; y < b_r.bottom; ++y) {
                for (int x = b_r.left; x < b_r.right; ++x) {
                    font._bitmap->decode(c.x + x-b_r.left, (c.y + y-b_r.top), font_col);
                    if(tint) {
                        font_col.r = (uint16_t (font_col.r)*color.r)>>r_;
                        font_col.g = (uint16_t (font_col.g)*color.g)>>g_;
                        font_col.b = (uint16_t (font_col.b)*color.b)>>b_;
                        font_col.a = (uint16_t (font_col.a)*color.a)>>a_;
                    }
                    blendColor<blendmode::Normal, porterduff::FastSourceOverOnOpaque, a_>(font_col, x, y, opacity);
//                    blendColor({255,0,0,128}, x, y, opacity);
                }
            }
        }
    }
    if(frame) {
        drawWuLine(color, left, top, left, bottom);
        drawWuLine(color, left, top, right, top);
        drawWuLine(color, right, top, right, bottom);
        drawWuLine(color, left, bottom, right, bottom);
    }
    updateClipRect(old.left, old.top, old.right, old.bottom);
}
