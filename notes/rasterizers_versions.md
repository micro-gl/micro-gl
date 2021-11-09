#### fast block rasterizer
```c++
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void canvas<P, CODER>::drawTriangle_internal(const color_f_t &color,
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
    vertex2<int> p_fixed = {minX<<sub_pixel_precision, minY<<sub_pixel_precision};
    vertex2<int> p = {minX , minY};

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
void canvas<P, CODER>::drawTriangle_internal(const color_f_t &color,
                                    const number v0_x, const number v0_y,
                                    const number v1_x, const number v1_y,
                                    const number v2_x, const number v2_y,
                                    const opacity_t opacity,
                                    bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    const precision precision = 8;
#define f_pos(v) microgl::math::to_fixed((v), precision)
    drawTriangle_internal<BlendMode, PorterDuff, antialias>(color,
            f_pos(v0_x), f_pos(v0_y), f_pos(v1_x), f_pos(v1_y), f_pos(v2_x), f_pos(v2_y),
            opacity, precision,
            aa_first_edge, aa_second_edge, aa_third_edge);
#undef f_pos
}

```
```c++
template<typename P, typename CODER>
template <typename BlendMode, typename PorterDuff, typename S>
void canvas<P, CODER>::drawRect_internal(const sampling::sampler<S> & sampler,
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
void canvas<P, CODER>::drawTriangle_internal(const sampling::sampler<S> &sampler,
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
    vertex2<l64> p = { minX, minY };
    vertex2<l64> p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };
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
```c++
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, typename S1, typename S2>
void canvas<P, CODER>::drawRoundedRect_internal(const sampling::sampler<S1> & sampler_fill,
                                       const sampling::sampler<S2> & sampler_stroke,
                                       int left, int top,
                                       int right, int bottom,
                                       int radius, int stroke_size,
                                       l64 u0, l64 v0, l64 u1, l64 v1,
                                       precision sub_pixel_precision, precision uv_p,
                                       canvas::opacity_t opacity) {
    const precision p = sub_pixel_precision;
    const l64 step = (l64(1)<<p);
    const l64 half = step>>1;
    const l64 stroke = stroke_size-step;//(10<<p)/1;
    const l64 aa_range = step;// (1<<p)/1;
    const l64 radius_squared=(l64(radius)*(radius))>>p;
    const l64 stroke_radius = (l64(radius-(stroke-0))*(radius-(stroke-0)))>>p;
    const l64 outer_aa_radius = (l64(radius+aa_range)*(radius+aa_range))>>p;
    const l64 outer_aa_bend = outer_aa_radius-radius_squared;
    const l64 inner_aa_radius = (l64(radius-(stroke-0)-aa_range)*(radius-(stroke-0)-aa_range))>>p;
    const l64 inner_aa_bend = stroke_radius-inner_aa_radius;
    const bool apply_opacity = opacity!=255;
    const l64 mask= (1<<sub_pixel_precision)-1;
    // dimensions in two spaces, one in raster spaces for optimization
    const l64 left_=(left+0), top_=(top+0), right_=(right), bottom_=(bottom);
    const l64 left_r=left_>>p, top_r=top_>>p, right_r=(right_+aa_range)>>p, bottom_r=(bottom_+aa_range)>>p;
    bool degenerate= left_r==right_r || top_r==bottom_r;
    if(degenerate) return;
    const l64 du = (u1-u0)/(right_r-left_r);
    const l64 dv = (v1-v0)/(bottom_r-top_r);
    // clipping
    const l64 left_r_c=functions::max<l64>(0, left_r), top_r_c=functions::max<l64>(0, top_r);
    const l64 right_r_c=functions::min<l64>(right_r, width()-1), bottom_r_c=functions::min<l64>(height()-1, bottom_r);
    const l64 dx=left_r_c-left_r, dy=top_r_c-top_r;
    color_t color;
    const int pitch = width();
    int index = top_r_c * pitch;
    for (l64 y_r=top_r_c, yy=top_&~mask+dy*step, v=v0+dy*dv; y_r<=bottom_r_c; y_r++, yy+=step, v+=dv, index+=pitch) {
        for (l64 x_r=left_r_c, xx=left_&~mask+dx*step, u=u0+dx*du; x_r<=right_r_c; x_r++, xx+=step, u+=du) {

            int blend_fill=opacity, blend_stroke=opacity;
            bool inside_radius;
            bool sample_fill=true, sample_stroke=false;
            const bool in_top_left= xx<=left_+radius && yy<=top_+radius;
            const bool in_bottom_left= xx<=left_+radius && yy>=bottom_-radius;
            const bool in_top_right= xx>=right_-radius && yy<=top_+radius;
            const bool in_bottom_right= xx>=right_-radius && yy>=bottom_-radius;
            const bool in_disks= in_top_left || in_bottom_left || in_top_right || in_bottom_right;

            if(in_disks) {
                l64 anchor_x=0, anchor_y=0;
                if(in_top_left) {anchor_x= left_+radius, anchor_y=top_+radius; }
                if(in_bottom_left) {anchor_x= left_+radius, anchor_y=bottom_-radius; }
                if(in_top_right) {anchor_x= right_-radius, anchor_y=top_+radius; }
                if(in_bottom_right) {anchor_x= right_-radius, anchor_y=bottom_-radius; }

                l64 dx = xx - anchor_x, dy = yy - anchor_y;
                const l64 distance_squared = ((l64(dx) * dx) >> p) + ((l64(dy) * dy) >> p);
                sample_fill=inside_radius = (distance_squared - radius_squared) <= 0;

                if (inside_radius) {
                    const bool inside_stroke = (distance_squared - stroke_radius) >= 0;
                    if (inside_stroke) { // inside stroke disk
                        blend_stroke = opacity;
                        sample_stroke=true;
                    }
                    else { // outside stroke disk, let's test for aa disk or radius inclusion
                        const l64 delta_inner_aa = -inner_aa_radius + distance_squared;
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
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x_r), blend_fill);
            }
            if (sample_stroke) {
                sampler_stroke.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x_r), blend_stroke);
            }
        }
    }
}

```

```c++
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number>
void canvas<P, CODER>::drawTriangle_shader_homo_internal(shader_base<impl, vertex_attr, varying, number> &shader,
                                                         const vertex4<number> &p0,  const vertex4<number> &p1,  const vertex4<number> &p2,
                                                         varying &varying_v0, varying &varying_v1, varying &varying_v2,
                                                         opacity_t opacity, const triangles::face_culling & culling,
                                                         long long * depth_buffer,
                                                         bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    /*
     * given triangle coords in a homogeneous coords, a shader, and corresponding interpolated varying
     * vertex attributes. we pass varying because somewhere in the pipeline we might have clipped things
     * in homogeneous space and therefore had to update/correct the vertex attributes.
     */
    const precision sub_pixel_precision = 8;
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
    vertex3<number> v0_viewport = {((v0_ndc.x + one)*w)/two, h - ((v0_ndc.y + one)*h)/two, (v0_ndc.z + one)/two};
    vertex3<number> v1_viewport = {((v1_ndc.x + one)*w)/two, h - ((v1_ndc.y + one)*h)/two, (v1_ndc.z + one)/two};
    vertex3<number> v2_viewport = {((v2_ndc.x + one)*w)/two, h - ((v2_ndc.y + one)*h)/two, (v2_ndc.z + one)/two};

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
#define ceil_fixed(val, bits) ((val)&((1<<bits)-1) ? ((val>>bits)+1) : (val>>bits))
#define floor_fixed(val, bits) ((val)>>bits)
    rect bbox;
    l64 mask = ~((l64(1)<<sub_pixel_precision)-1);
    l64 minX = floor_fixed(functions::min<l64>(v0_x, v1_x, v2_x)&mask, sub_pixel_precision);
    l64 minY = floor_fixed(functions::min<l64>(v0_y, v1_y, v2_y)&mask, sub_pixel_precision);
    l64 maxX = ceil_fixed(functions::max<l64>(v0_x, v1_x, v2_x), sub_pixel_precision);
    l64 maxY = ceil_fixed(functions::max<l64>(v0_y, v1_y, v2_y), sub_pixel_precision);
    // clipping
    minX = functions::max<l64>(0, minX); minY = functions::max<l64>(0, minY);
    maxX = functions::min<l64>(width()-1, maxX); maxY = functions::min<l64>(height()-1, maxY);
#undef ceil_fixed
#undef floor_fixed
    bool outside= maxX<0 || maxY<0 || minX>(width()-1) || minY>(height()-1);
    if(outside) return; // cull in 2d raster window
    constexpr int max_alpha_value= bitmap::maxNativeAlphaChannelValue();
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
    vertex2<l64> p = { minX, minY };
    vertex2<l64> p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };
    l64 half= (l64(1)<<(sub_pixel_precision))>>1;
    p_fixed = p_fixed + vertex2<l64> {half, half}; // we sample at the center
    // this can produce a 2P bits number if the points form a a perpendicular triangle
    // this is my patent for correct fill rules without wasting bits, amazingly works and accurate,
    // I still need to explain to myself why it works so well :)
    l64 w0_row = functions::orient2d(v0_x, v0_y, v1_x, v1_y, p_fixed.x, p_fixed.y, 0) + bias_w0;
    l64 w1_row = functions::orient2d(v1_x, v1_y, v2_x, v2_y, p_fixed.x, p_fixed.y, 0) + bias_w1;
    l64 w2_row = functions::orient2d(v2_x, v2_y, v0_x, v0_y, p_fixed.x, p_fixed.y, 0) + bias_w2;
    w0_row = w0_row>>sub_pixel_precision; w1_row = w1_row>>sub_pixel_precision; w2_row = w2_row>>sub_pixel_precision;
    // Triangle setup, this needs at least (P+1) bits, since the delta is always <= length
    int64_t A01 = (v0_y - v1_y), B01 = (v1_x - v0_x);
    int64_t A12 = (v1_y - v2_y), B12 = (v2_x - v1_x);
    int64_t A20 = (v2_y - v0_y), B20 = (v0_x - v2_x);
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
    const int pitch= width();
    int index = p.y * pitch;
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
            auto bary = vertex4<l64>{w0, w1, w2, area};
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
                // test_texture edges
                if(!perform_aa) {
                    if(distance==w0_h && aa_first_edge) perform_aa = true;
                    else if(distance==w1_h && aa_second_edge) perform_aa = true;
                    else perform_aa = distance == w2_h && aa_third_edge;
                }
                should_sample= perform_aa && delta>=0;
                if(should_sample) {
                    opacity_t blend = functions::clamp<int64_t>((((int64_t(delta) << bits_distance_complement))>>PREC_DIST),
                                                                0, 255);
                    if (opacity < max_alpha_value)
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
                // cast to user's number types vertex4<number> casted_bary= bary;, I decided to stick with l64
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
        index += pitch;
    }
}

```

```c++
template<typename bitmap_, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, typename S>
void canvas<bitmap_, options>::drawTriangle_internal(const sampling::sampler<S> &sampler,
                                  int v0_x, int v0_y, int u0, int v0, int q0,
                                  int v1_x, int v1_y, int u1, int v1, int q1,
                                  int v2_x, int v2_y, int u2, int v2, int q2,
                                  const opacity_t opacity, const precision sub_pixel_precision,
                                  const precision uv_precision, bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
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
#define floor_fixed(val, bits) ((val)>>bits)
    rect bbox;
    l64 mask = ~((l64(1)<<sub_pixel_precision)-1);
    bbox.left = floor_fixed(functions::min<l64>(v0_x, v1_x, v2_x)&mask, sub_pixel_precision);
    bbox.top = floor_fixed(functions::min<l64>(v0_y, v1_y, v2_y)&mask, sub_pixel_precision);
    bbox.right = ceil_fixed(functions::max<l64>(v0_x, v1_x, v2_x), sub_pixel_precision);
    bbox.bottom = ceil_fixed(functions::max<l64>(v0_y, v1_y, v2_y), sub_pixel_precision);
    // clipping
    bbox = bbox.intersect(effectiveRect);
#undef ceil_fixed
#undef floor_fixed
//bbox.right=319;
    // anti-alias pad for distance calculation
    constexpr int max_opacity_value= 255;//bitmap::maxNativeAlphaChannelValue();
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
        bits_distance = 6;
        bits_distance_complement = 8 - bits_distance;
        max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias<<PREC_DIST;
    }
    // fill rules adjustments
    triangles::top_left_t top_left =
            triangles::classifyTopLeftEdges(false, v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    int bias_w0 = top_left.first  ? 0 : -(1);
    int bias_w1 = top_left.second ? 0 : -(1);
    int bias_w2 = top_left.third  ? 0 : -(1);
    // Barycentric coordinates at minX/minY corner
    vertex2<l64> p = { bbox.left, bbox.top };
    vertex2<l64> p_fixed = { bbox.left<<sub_pixel_precision, bbox.top<<sub_pixel_precision };
    l64 half= (l64(1)<<(sub_pixel_precision))>>1;
    p_fixed = p_fixed + vertex2<l64> {half, half}; // we sample at the center
    ///
    int bits_used=0;
    { while (int(area)>int(1<<(bits_used++))) {
        int xxx=0;;
    }; --bits_used; }

    // this can produce a 2P bits number if the points form a a perpendicular triangle
    // this is my patent for correct fill rules without wasting bits, amazingly works and accurate,
    // I still need to exaplin to myself why it works so well :)
    l64 w0_row = functions::orient2d(v0_x, v0_y, v1_x, v1_y, p_fixed.x, p_fixed.y, 0) + bias_w0;
    l64 w1_row = functions::orient2d(v1_x, v1_y, v2_x, v2_y, p_fixed.x, p_fixed.y, 0) + bias_w1;
    l64 w2_row = functions::orient2d(v2_x, v2_y, v0_x, v0_y, p_fixed.x, p_fixed.y, 0) + bias_w2;
    w0_row = w0_row>>sub_pixel_precision; w1_row = w1_row>>sub_pixel_precision; w2_row = w2_row>>sub_pixel_precision;
    // sub_pixel_precision;
    const precision BITS_UV_COORDS = uv_precision;
    const precision PP = sub_pixel_precision;
    uint8_t MAX_PREC = 63;
    uint8_t LL = bits_used+12;//MAX_PREC - (sub_pixel_precision + BITS_UV_COORDS+10);
//    uint8_t LL = MAX_PREC - (sub_pixel_precision + BITS_UV_COORDS+0);
    l64 ONE = ((l64)1)<<LL; // area is 2b+2p-p=2b+p bits
//    l64 ONE = ((l64)1)<<32;
    l64 one_area = (ONE) / area;
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
    const int pitch= width();
    int index = p.y * pitch;
    for (p.y = bbox.top; p.y <= bbox.bottom; p.y++) {
        l64 w0 = w0_row;
        l64 w1 = w1_row;
        l64 w2 = w2_row;
        l64 w0_h=0,w1_h=0,w2_h=0;
        if(antialias) {
            w0_h = w0_row_h;
            w1_h = w1_row_h;
            w2_h = w2_row_h;
        }
        for (p.x = bbox.left; p.x <= bbox.right; p.x++) {
            bool should_sample=false;
            uint8_t blend=opacity;
            if((w0|w1|w2)>=0) should_sample=true;
            else if(antialias) { // cheap AA based on SDF
#define G
                const l64 distance = functions::min(G(w0_h), G(w1_h), G(w2_h));
                l64 delta = distance+max_distance_scaled_space_anti_alias;
                bool perform_aa = delta>=0 && (aa_all_edges || ((distance == G(w0_h)) && aa_first_edge) ||
                                               ((distance == G(w1_h)) && aa_second_edge) ||
                                               ((distance == G(w2_h)) && aa_third_edge));
                if (perform_aa) {
                    should_sample = true;
                    blend = functions::clamp<int>(((uint64_t)(delta << bits_distance_complement))>>PREC_DIST,0, 255);
                    if (opacity < max_opacity_value) blend = (blend * opacity) >> 8; // * 257>>16 - blinn method
                }
            }

            if(should_sample) {
                l64 u_i, v_i;
                // I compress down the weights to save some bits
//                l64 u_fixed = ((w0*u2)>>PP) + ((w1*u0)>>PP) + ((w2*u1)>>PP);
//                l64 v_fixed = ((w0*v2)>>PP) + ((w1*v0)>>PP) + ((w2*v1)>>PP);
                l64 u_fixed = (w0*u2 + w1*u0 + w2*u1)>>PP;
                l64 v_fixed = (w0*v2 + w1*v0 + w2*v1)>>PP;
                if(perspective_correct) {
                    auto q_fixed = ((w0*q2)>>PP) + ((w1*q0)>>PP) + ((w2*q1)>>PP);
                    u_i = (u_fixed<<BITS_UV_COORDS)/q_fixed;
                    v_i = (v_fixed<<BITS_UV_COORDS)/q_fixed;
                } else {
                    // stabler rasterizer, that will not overflow fast
//                    u_i = (u_fixed)/(area>>PP);
//                    v_i = (v_fixed)/(area>>PP);
                    u_i = ((u_fixed*one_area)>>(LL-PP));
                    v_i = (v_fixed*one_area)>>(LL-PP);
//                    u_i = ((u_fixed*one_area)>>(LL));
//                    v_i = (v_fixed*one_area)>>(LL);
                }
                if(antialias) {
                    u_i = functions::clamp<l64>(u_i, 0, (1ll<<BITS_UV_COORDS));
                    v_i = functions::clamp<l64>(v_i, 0, (1ll<<BITS_UV_COORDS));
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
        index += pitch;
    }
}
```
```c++
template<typename bitmap_, uint8_t options>
template <typename BlendMode, typename PorterDuff, bool antialias, typename S>
void canvas<bitmap_, options>::drawRect_internal(const sampling::sampler<S> & sampler,
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
    const int du = (u1-u0)/(bbox_r.right-bbox_r.left-0);
    const int dv = (v1-v0)/(bbox_r.bottom-bbox_r.top-0);
    const int dx= bbox_r_c.left-bbox_r.left, dy= bbox_r_c.top-bbox_r.top;
    u0+=du>>1; v0+=dv>>1; // sample from_sampler the middle always for best results
    const int u_start= u0+dx*du;
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
        for (int y=bbox_r_c.top, v=v0+dy*dv; y<=bbox_r_c.bottom; y++, v+=dv, index+=pitch) {
            for (int x=bbox_r_c.left, u=u_start; x<=bbox_r_c.right; x++, u+=du) {
                blend=opacity;
                if(x==bbox_r_c.left && !clipped_left) {
                    if(y==bbox_r_c.top && !clipped_top)
                        blend= blend_left_top;
                    else if(y==bbox_r_c.bottom && !clipped_bottom)
                        blend= blend_left_bottom;
                    else blend= blend_left;
                }
                else if(x==bbox_r_c.right && !clipped_right) {
                    if(y==bbox_r_c.top && !clipped_top)
                        blend= blend_right_top;
                    else if(y==bbox_r_c.bottom && !clipped_bottom)
                        blend= blend_right_bottom;
                    else
                        blend= blend_right;
                }
                else if(y==bbox_r_c.top && !clipped_top)
                    blend= blend_top;
                else if(y==bbox_r_c.bottom && !clipped_bottom)
                    blend= blend_bottom;

                sampler.sample(u, v, uv_precision, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + x, blend);
            }
        }
    }
    else {
        int index= bbox_r_c.top * pitch;
        for (int y=bbox_r_c.top, v=v0+dy*dv; y<bbox_r_c.bottom; y++, v+=dv, index+=pitch) {
            for (int x=bbox_r_c.left, u=u_start; x<bbox_r_c.right; x++, u+=du) {
                sampler.sample(u, v, uv_precision, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + x, opacity);
            }
        }
    }
#undef ceil_fixed
#undef floor_fixed
}
```
```c++
template<typename bitmap_, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, typename S1, typename S2>
void canvas<bitmap_, options>::drawRoundedRect_internal(const sampling::sampler<S1> & sampler_fill,
                                     const sampling::sampler<S2> & sampler_stroke,
                                     int left, int top,
                                     int right, int bottom,
                                     int radius, int stroke_size,
                                     l64 u0, l64 v0, l64 u1, l64 v1,
                                     precision sub_pixel_precision, precision uv_p,
                                     canvas::opacity_t opacity) {
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
    const precision p = sub_pixel_precision;
    const l64 step = (l64(1)<<p);
    const l64 half = step>>1;
    const l64 stroke = stroke_size-step;//(10<<p)/1;
    const l64 aa_range = step;// (1<<p)/1;
    const l64 radius_squared=(l64(radius)*(radius))>>p;
    const l64 stroke_radius = (l64(radius-(stroke-0))*(radius-(stroke-0)))>>p;
    const l64 outer_aa_radius = (l64(radius+aa_range)*(radius+aa_range))>>p;
    const l64 outer_aa_bend = outer_aa_radius-radius_squared;
    const l64 inner_aa_radius = (l64(radius-(stroke-0)-aa_range)*(radius-(stroke-0)-aa_range))>>p;
    const l64 inner_aa_bend = stroke_radius-inner_aa_radius;
    const bool apply_opacity = opacity!=255;
    const l64 mask= (1<<sub_pixel_precision)-1;
    // dimensions in two spaces, one in raster spaces for optimization
    const l64 left_=(left+0), top_=(top+0), right_=(right), bottom_=(bottom);
    const rect bbox_r = {left_>>p, top_>>p,(right_+aa_range)>>p, (bottom_+aa_range)>>p};
    const rect bbox_r_c = bbox_r.intersect(effectiveRect);
    if(bbox_r_c.empty()) return;
    const l64 du = (u1-u0)/(bbox_r.right-bbox_r.left);
    const l64 dv = (v1-v0)/(bbox_r.bottom-bbox_r.top);
    const l64 dx=bbox_r_c.left-bbox_r.left, dy=bbox_r_c.top-bbox_r.top;
    color_t color;
    const int pitch = width();
    int index = bbox_r_c.top * pitch;
    for (l64 y_r=bbox_r_c.top, yy=(top_&~mask)+dy*step, v=v0+dy*dv; y_r<=bbox_r_c.bottom; y_r++, yy+=step, v+=dv, index+=pitch) {
        for (l64 x_r=bbox_r_c.left, xx=(left_&~mask)+dx*step, u=u0+dx*du; x_r<=bbox_r_c.right; x_r++, xx+=step, u+=du) {

            int blend_fill=opacity, blend_stroke=opacity;
            bool inside_radius;
            bool sample_fill=true, sample_stroke=false;
            const bool in_top_left= xx<=left_+radius && yy<=top_+radius;
            const bool in_bottom_left= xx<=left_+radius && yy>=bottom_-radius;
            const bool in_top_right= xx>=right_-radius && yy<=top_+radius;
            const bool in_bottom_right= xx>=right_-radius && yy>=bottom_-radius;
            const bool in_disks= in_top_left || in_bottom_left || in_top_right || in_bottom_right;

            if(in_disks) {
                l64 anchor_x=0, anchor_y=0;
                if(in_top_left) {anchor_x= left_+radius, anchor_y=top_+radius; }
                if(in_bottom_left) {anchor_x= left_+radius, anchor_y=bottom_-radius; }
                if(in_top_right) {anchor_x= right_-radius, anchor_y=top_+radius; }
                if(in_bottom_right) {anchor_x= right_-radius, anchor_y=bottom_-radius; }

                l64 delta_x = xx - anchor_x, delta_y = yy - anchor_y;
                const l64 distance_squared = ((l64(delta_x) * delta_x) >> p) + ((l64(delta_y) * delta_y) >> p);
                sample_fill=inside_radius = (distance_squared - radius_squared) <= 0;

                if (inside_radius) {
                    const bool inside_stroke = (distance_squared - stroke_radius) >= 0;
                    if (inside_stroke) { // inside stroke disk
                        blend_stroke = opacity;
                        sample_stroke=true;
                    }
                    else { // outside stroke disk, let's test_texture for aa disk or radius inclusion
                        const l64 delta_inner_aa = -inner_aa_radius + distance_squared;
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
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x_r), blend_fill);
            }
            if (sample_stroke) {
                sampler_stroke.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x_r), blend_stroke);
            }
        }
    }
}

```

```c++
template<typename bitmap_, uint8_t options>
template<typename BlendMode, typename PorterDuff, bool antialias, bool perspective_correct, bool depth_buffer_flag,
        typename impl, typename vertex_attr, typename varying, typename number>
void canvas<bitmap_, options>::drawTriangle_shader_homo_internal(shader_base<impl, vertex_attr, varying, number> &shader,
                                                         int viewport_width, int viewport_height,
                                                         const vertex4<number> &p0,  const vertex4<number> &p1,  const vertex4<number> &p2,
                                                         varying &varying_v0, varying &varying_v1, varying &varying_v2,
                                                         opacity_t opacity, const triangles::face_culling & culling,
                                                         long long * depth_buffer) {
    /*
     * given triangle coords in a homogeneous coords, a shader, and corresponding interpolated varying
     * vertex attributes. we pass varying because somewhere in the pipeline we might have clipped things
     * in homogeneous space and therefore had to update/correct the vertex attributes.
     */
    auto effectiveRect = calculateEffectiveDrawRect();
    if(effectiveRect.empty()) return;
    const precision sub_pixel_precision = 8;
#define f microgl::math::to_fixed
    varying interpolated_varying;
    // perspective divide by w -> NDC space
    if(p0.w==0 || p1.w==0 || p2.w==0) return;
    const auto v0_ndc = p0/p0.w;
    const auto v1_ndc = p1/p1.w;
    const auto v2_ndc = p2/p2.w;
    // viewport transform: NDC space -> raster space
    const number w= viewport_width;
    const number h= viewport_height;
    number one = number(1), two=number(2);
    vertex3<number> v0_viewport = {((v0_ndc.x + one)*w)/two, h - ((v0_ndc.y + one)*h)/two, (v0_ndc.z + one)/two};
    vertex3<number> v1_viewport = {((v1_ndc.x + one)*w)/two, h - ((v1_ndc.y + one)*h)/two, (v1_ndc.z + one)/two};
    vertex3<number> v2_viewport = {((v2_ndc.x + one)*w)/two, h - ((v2_ndc.y + one)*h)/two, (v2_ndc.z + one)/two};
    // collect values for interpolation as fixed point integers
    int v0_x= f(v0_viewport.x, sub_pixel_precision), v0_y= f(v0_viewport.y, sub_pixel_precision);
    int v1_x= f(v1_viewport.x, sub_pixel_precision), v1_y= f(v1_viewport.y, sub_pixel_precision);
    int v2_x= f(v2_viewport.x, sub_pixel_precision), v2_y= f(v2_viewport.y, sub_pixel_precision);
    //
    auto bits_w0=microgl::functions::used_integer_bits(f(p0.w, sub_pixel_precision));
    auto bits_w1=microgl::functions::used_integer_bits(f(p1.w, sub_pixel_precision));
    auto bits_w2=microgl::functions::used_integer_bits(f(p2.w, sub_pixel_precision));
    //
    const int w_bits= 18; const l64 one_w= (l64(1) << (w_bits << 1)); // negate z because camera is looking negative z axis
    l64 v0_w= one_w / f(p0.w, w_bits), v1_w= one_w / f(p1.w, w_bits), v2_w= one_w / f(p2.w, w_bits);
    const int z_bits= 24; const l64 one_z= (l64(1) << (z_bits)); // negate z because camera is looking negative z axis
    l64 v0_z= f(v0_viewport.z, z_bits), v1_z= f(v1_viewport.z, z_bits), v2_z= f(v2_viewport.z, z_bits);

    l64 area = functions::orient2d<l64, l64>(v0_x, v0_y, v1_x, v1_y, v2_x, v2_y, sub_pixel_precision);
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
        functions::swap(v1_w, v2_w); functions::swap(v1_z, v2_z);
    }
    // rotate to match edges
    functions::swap(varying_v0, varying_v1);
    functions::swap(v0_w, v1_w); functions::swap(v0_z, v1_z);

#undef f
    // bounding box in raster space
#define ceil_fixed(val, bits) ((val)&((1<<bits)-1) ? ((val>>bits)+1) : (val>>bits))
#define floor_fixed(val, bits) ((val)>>bits)
    rect bbox;
    l64 mask = ~((l64(1)<<sub_pixel_precision)-1);
    bbox.left = floor_fixed(functions::min<l64>(v0_x, v1_x, v2_x)&mask, sub_pixel_precision);
    bbox.top = floor_fixed(functions::min<l64>(v0_y, v1_y, v2_y)&mask, sub_pixel_precision);
    bbox.right = ceil_fixed(functions::max<l64>(v0_x, v1_x, v2_x), sub_pixel_precision);
    bbox.bottom = ceil_fixed(functions::max<l64>(v0_y, v1_y, v2_y), sub_pixel_precision);
    bbox = bbox.intersect(effectiveRect);
    if(bbox.empty()) return;
#undef ceil_fixed
#undef floor_fixed
    // fill rules configurations
    triangles::top_left_t top_left =
            triangles::classifyTopLeftEdges(false,
                                            v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);
    int bias_w0 = top_left.first  ? 0 : -1;
    int bias_w1 = top_left.second ? 0 : -1;
    int bias_w2 = top_left.third  ? 0 : -1;
    // Barycentric coordinates at minX/minY corner
    vertex2<l64> p = { bbox.left, bbox.top };
    vertex2<l64> p_fixed = { bbox.left<<sub_pixel_precision, bbox.top<<sub_pixel_precision };
    l64 half= (l64(1)<<(sub_pixel_precision))>>1;
    p_fixed = p_fixed + vertex2<l64> {half, half}; // we sample at the center
    // this can produce a 2P bits number if the points form a a perpendicular triangle
    // this is my patent for correct fill rules without wasting bits, amazingly works and accurate,
    // I still need to explain to myself why it works so well :)
    l64 w0_row = functions::orient2d<int, l64>(v0_x, v0_y, v1_x, v1_y, p_fixed.x, p_fixed.y, 0) + bias_w0;
    l64 w1_row = functions::orient2d<int, l64>(v1_x, v1_y, v2_x, v2_y, p_fixed.x, p_fixed.y, 0) + bias_w1;
    l64 w2_row = functions::orient2d<int, l64>(v2_x, v2_y, v0_x, v0_y, p_fixed.x, p_fixed.y, 0) + bias_w2;
    w0_row = w0_row>>sub_pixel_precision; w1_row = w1_row>>sub_pixel_precision; w2_row = w2_row>>sub_pixel_precision;
    // Triangle setup, this needs at least (P+1) bits, since the delta is always <= length
    int64_t A01 = (v0_y - v1_y), B01 = (v1_x - v0_x);
    int64_t A12 = (v1_y - v2_y), B12 = (v2_x - v1_x);
    int64_t A20 = (v2_y - v0_y), B20 = (v0_x - v2_x);
    const int pitch= width(); int index = p.y * pitch;
    for (p.y = bbox.top; p.y <= bbox.bottom; p.y++, index+=pitch) {
        int w0 = w0_row, w1 = w1_row, w2 = w2_row;
        for (p.x = bbox.left; p.x<=bbox.right; p.x++) {
            const bool in_closure= (w0|w1|w2)>=0;
            bool should_sample= in_closure;
            auto opacity_sample = opacity;
            auto bary = vertex4<l64>{w0, w1, w2, area};
            if(in_closure && perspective_correct) { // compute perspective-correct and transform to sub-pixel-space
                bary.x= (l64(w0)*v0_w)>>w_bits, bary.y= (l64(w1)*v1_w)>>w_bits, bary.z= (l64(w2)*v2_w)>>w_bits;
                bary.w=bary.x+bary.y+bary.z;
                if(bary.w==0) bary.w=1;
            }
            if(depth_buffer_flag && should_sample) {
                l64 z;
                constexpr bool is_float_point=microgl::traits::is_float_point<number>();
                // take advantage of FPU
                if(is_float_point) z= (long long)(number((v0_z*w0) +(v1_z*w1) +(v2_z*w2))/(area));
                else z= (((v0_z)*bary.x) +((v1_z)*bary.y) +((v2_z)*bary.z))/(bary.w);
                //z_tag= functions::clamp<l64>(z_tag, 0, l64(1)<<44);
                const int z_index = index - _window.index_correction + p.x;
                if(z<0 || z>depth_buffer[z_index]) should_sample=false;
                else depth_buffer[z_index]=z;
            }
            if(should_sample) {
                // cast to user's number types vertex4<number> casted_bary= bary;, I decided to stick with l64
                // because other wise this would have wasted bits for Q types although it would have been more elegant.
                interpolated_varying.interpolate(varying_v0, varying_v1, varying_v2, bary);
                auto color = shader.fragment(interpolated_varying);
                blendColor<BlendMode, PorterDuff>(color, index + p.x, opacity_sample);
            }
            w0+=A01; w1+=A12; w2+=A20;
        }
        w0_row+=B01; w1_row+=B12; w2_row+=B20;
    }
}

```