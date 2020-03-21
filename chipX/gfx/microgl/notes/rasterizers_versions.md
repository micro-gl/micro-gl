#### fast block rasterizer
```c++
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
    int sign = functions::orient2d(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, sub_pixel_precision);
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
    }
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
    const int block = 4;
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
    vec2<int> p_fixed = {minX<<sub_pixel_precision, minY<<sub_pixel_precision};
    vec2<int> p = {minX , minY};

    int w0_row = functions::orient2d(v0_x, v0_y, v1_x, v1_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w0;
    int w1_row = functions::orient2d(v1_x, v1_y, v2_x, v2_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w1;
    int w2_row = functions::orient2d(v2_x, v2_y, v0_x, v0_y, p_fixed.x, p_fixed.y, sub_pixel_precision) + bias_w2;

    // AA, 2A/L = h, therefore the division produces a P bit number
    int w0_row_h=0, w1_row_h=0, w2_row_h=0;
    int A01_h=0, B01_h=0, A12_h=0, B12_h=0, A20_h=0, B20_h=0;
    int A01_block_h=0, B01_block_h=0, A12_block_h=0, B12_block_h=0, A20_block_h=0, B20_block_h=0;
    int A01_block_m_1_h=0, B01_block_m_1_h=0, A12_block_m_1_h=0, B12_block_m_1_h=0, A20_block_m_1_h=0, B20_block_m_1_h=0;
    if(antialias) {
        int PP = PR;
        // lengths of edges
        unsigned int length_w0 = microgl::math::distance(v0_x, v0_y, v1_x, v1_y);
        unsigned int length_w1 = microgl::math::distance(v1_x, v1_y, v2_x, v2_y);
        unsigned int length_w2 = microgl::math::distance(v0_x, v0_y, v2_x, v2_y);
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
                        blendColor<BlendMode, PorterDuff>(color_int, stride+ix, opacity);
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
                            if ((w0_|w1_|w2_)>=0)
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
                                if (perform_aa && delta>=0) { // take the complement and rescale
                                    precision blend = functions::clamp<int>(((int64_t)delta << bits_distance_complement)>>(PR),
                                                                          0, 255);
                                    if (perform_opacity)
                                        blend = (blend * opacity) >> 8;
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
    const precision precision = 8;
#define f_pos(v) microgl::math::to_fixed((v), precision)
    drawTriangle<BlendMode, PorterDuff, antialias>(color,
            f_pos(v0_x), f_pos(v0_y), f_pos(v1_x), f_pos(v1_y), f_pos(v2_x), f_pos(v2_y),
            opacity, precision,
            aa_first_edge, aa_second_edge, aa_third_edge);
#undef f_pos
}

```
```c++
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
```

```c++

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, typename S>
void Canvas<P, CODER>::drawTriangle(const sampling::sampler<S> &sampler,
                                    int v0_x, int v0_y, int u0, int v0, int q0,
                                    int v1_x, int v1_y, int u1, int v1, int q1,
                                    int v2_x, int v2_y, int u2, int v2, int q2,
                                    const opacity_t opacity, const precision sub_pixel_precision,
                                    const precision uv_precision, bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
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

    // bounding box
#define ceil_fixed(val, bits) ((val)&((1<<bits)-1) ? ((val>>bits)+1) : (val>>bits))
#define floor_fixed(val, bits) (val>>bits)
    l64 minX = floor_fixed(functions::min(v0_x, v1_x, v2_x), sub_pixel_precision);
    l64 minY = floor_fixed(functions::min(v0_y, v1_y, v2_y), sub_pixel_precision);
    l64 maxX = ceil_fixed(functions::max(v0_x, v1_x, v2_x), sub_pixel_precision);
    l64 maxY = ceil_fixed(functions::max(v0_y, v1_y, v2_y), sub_pixel_precision);
//    l64 max_sub_pixel_precision_value = (l64(1)<<sub_pixel_precision) - 1;
//    l64 minX = (functions::min(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
//    l64 minY = (functions::min(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;
//    l64 maxX = (functions::max(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
//    l64 maxY = (functions::max(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;
#undef ceil_fixed
#undef floor_fixed
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
        A01_h = (((l64)(v0_y - v1_y))<<PREC_DIST)/length_w0, B01_h = (((l64)(v1_x - v0_x))<<PREC_DIST)/length_w0;
        A12_h = (((l64)(v1_y - v2_y))<<PREC_DIST)/length_w1, B12_h = (((l64)(v2_x - v1_x))<<PREC_DIST)/length_w1;
        A20_h = (((l64)(v2_y - v0_y))<<PREC_DIST)/length_w2, B20_h = (((l64)(v0_x - v2_x))<<PREC_DIST)/length_w2;
        w0_row_h = (((l64)(w0_row))<<PREC_DIST)/length_w0;
        w1_row_h = (((l64)(w1_row))<<PREC_DIST)/length_w1;
        w2_row_h = (((l64)(w2_row))<<PREC_DIST)/length_w2;
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
```