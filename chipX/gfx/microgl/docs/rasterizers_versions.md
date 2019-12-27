### a simple color rasterizer
a fast rasterizer, includes fill rules, AA and integer iterations, but does not includes
block mode, which can double the performance for just plotting colors, but is more complicated
and long to implement. This version is good for reference.

```
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void Canvas<P, CODER>::drawTriangle(const color_f_t &color,
                                    const fixed_signed v0_x, const fixed_signed v0_y,
                                    const fixed_signed v1_x, const fixed_signed v1_y,
                                    const fixed_signed v2_x, const fixed_signed v2_y,
                                    const uint8_t opacity,
                                    const uint8_t sub_pixel_precision,
                                    bool aa_first_edge,
                                    bool aa_second_edge,
                                    bool aa_third_edge) {
    color_t color_int;
    coder()->convert(color, color_int);

    // sub_pixel_precision;
    // THIS MAY HAVE TO BE MORE LIKE 15 TO AVOID OVERFLOW
    uint8_t MAX_BITS_FOR_PROCESSING_PRECISION = 15;
    uint8_t PR = MAX_BITS_FOR_PROCESSING_PRECISION;// - sub_pixel_precision;
    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    // bbox
    int minX = (functions::min(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int minY = (functions::min(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxX = (functions::max(v0_x, v1_x, v2_x) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxY = (functions::max(v0_y, v1_y, v2_y) + max_sub_pixel_precision_value) >> sub_pixel_precision;

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

    // fill rules adjustments
    triangles::top_left_t top_left =
            triangles::classifyTopLeftEdges(false,
                                            v0_x, v0_y, v1_x, v1_y, v2_x, v2_y);

    int bias_w0 = top_left.first  ? 0 : -1;
    int bias_w1 = top_left.second ? 0 : -1;
    int bias_w2 = top_left.third  ? 0 : -1;
    //

    minX = functions::max(0, minX); minY = functions::max(0, minY);
    maxX = functions::min((width()-1), maxX); maxY = functions::min((height()-1), maxY);

    // Triangle setup
    int A01 = v0_y - v1_y, B01 = v1_x - v0_x;
    int A12 = v1_y - v2_y, B12 = v2_x - v1_x;
    int A20 = v2_y - v0_y, B20 = v0_x - v2_x;

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
    if(antialias) {
        int PP = PR;

        // lengths of edges
        unsigned int length_w0 = functions::length({v0_x, v0_y}, {v1_x, v1_y}, 0);
        unsigned int length_w1 = functions::length({v1_x, v1_y}, {v2_x, v2_y}, 0);
        unsigned int length_w2 = functions::length({v0_x, v0_y}, {v2_x, v2_y}, 0);

        A01_h = (((int64_t)(v0_y - v1_y))<<(PP))/length_w0, B01_h = (((int64_t)(v1_x - v0_x))<<(PP))/length_w0;
        A12_h = (((int64_t)(v1_y - v2_y))<<(PP))/length_w1, B12_h = (((int64_t)(v2_x - v1_x))<<(PP))/length_w1;
        A20_h = (((int64_t)(v2_y - v0_y))<<(PP))/length_w2, B20_h = (((int64_t)(v0_x - v2_x))<<(PP))/length_w2;

        w0_row_h = (((int64_t)(w0_row))<<(PP))/length_w0;
        w1_row_h = (((int64_t)(w1_row))<<(PP))/length_w1;
        w2_row_h = (((int64_t)(w2_row))<<(PP))/length_w2;
    }

    /*
    //
    // distance to edge is always h= (2*A)/L, where:
    // h=distance from point to edge
    // A = triangle area spanned by point and edge area
    // L = length of the edge
    // this simple geometric identity can be derived from
    // area of triangle equation. We are going to interpolate
    // the quantity h and we would like to evaluate h.
    // NOTE:: this is a cheap way to calculate anti-alias with
    // perpendicular distance, this is of course not correct for
    // points that are "beyond" the edges. The real calculation
    // has to use distance to points hence a square root function
    // which is expensive for integer version. This version seems to
    // work best with minimal artifacts when used with bits_distance=0 or 1.
    //
    // we interpolate in scaled precision so watch out. All of the working
    // calculations are in 15 or 16 bits precision.

     */

    // watch out for negative values
    int index = p.y * (_width);

    for (p.y = minY; p.y <= maxY; p.y+=1) {

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

        for (p.x = minX; p.x <= maxX; p.x+=1) {

            if ((w0 | w1 | w2) >= 0) {

                blendColor<BlendMode, PorterDuff>(color_int, (index + p.x), opacity);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                // find minimal distance along edges only, this does not take
                // into account the junctions
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

                if (perform_aa && delta>=0) {

                    // take the complement and rescale
                    uint8_t blend = functions::clamp<int>(((int64_t)delta << bits_distance_complement)>>(PR),
                                                          0, 255);

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
                    }
//                    blend=255;
                    blendColor<BlendMode, PorterDuff>(color_int, (index + p.x), blend);
                }

            }

            // One step to the right
            w0 += A01;
            w1 += A12;
            w2 += A20;

            if(antialias) {
                w0_h += A01_h;
                w1_h += A12_h;
                w2_h += A20_h;
            }

        }

        // One row step
        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        if(antialias) {
            w0_row_h += B01_h;
            w1_row_h += B12_h;
            w2_row_h += B20_h;
        }

        index += (_width);
    }

}


```


#### quadrilateral computation completely with fixed points

```
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, typename Sampler,
        typename P2, typename CODER2>
void
Canvas<P, CODER>::drawQuadrilateral(const Bitmap<P2, CODER2> & bmp,
                                    int v0_x, int v0_y, int u0, int v0,
                                    int v1_x, int v1_y, int u1, int v1,
                                    int v2_x, int v2_y, int u2, int v2,
                                    int v3_x, int v3_y, int u3, int v3,
                                    const uint8_t opacity, const precision sub_pixel_precision,
                                    const precision uv_precision) {

    int q_one = 1<<uv_precision;

//    bool isParallelogram_ = isParallelogram(p0, p1, p2, p3);
    bool isParallelogram_ = functions::isParallelogram({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, {v3_x, v3_y});

    if(isParallelogram_) {

        if(functions::isAxisAlignedRectangle({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, {v3_x, v3_y})) {
            int left = functions::min(v0_x, v1_x, v2_x, v3_x);
            int top = functions::min(v0_y, v1_y, v2_y, v3_y);
            int right = functions::max(v0_x, v1_x, v2_x, v3_x);
            int bottom = functions::max(v0_y, v1_y, v2_y, v3_y);
            int u0_ = functions::min(u0, u1, u2, u3);
            int v0_ = functions::max(v0, v1, v2, v3);
            int u1_ = functions::max(0, u1, u2, u3);
            int v1_ = functions::min(v0, v1, v2, v3);

            drawQuad<BlendMode, PorterDuff, Sampler>(bmp, left, top, right, bottom, u0_, v0_, u1_, v1_,
                    sub_pixel_precision, uv_precision, opacity);

            return;
        }

        // Note:: this was faster than rasterizing the two triangles
        // in the same loop for some reason.
        drawTriangle<BlendMode, PorterDuff, antialias, false, Sampler>(bmp,
                                                              v0_x, v0_y, u0, v0, q_one,
                                                              v1_x, v1_y, u1, v1, q_one,
                                                              v2_x, v2_y, u2, v2, q_one,
                                                              opacity, sub_pixel_precision,
                                                              uv_precision,
                                                              true, true, false);

        drawTriangle<BlendMode, PorterDuff, antialias, false, Sampler>(bmp,
                                                              v2_x, v2_y, u2, v2, q_one,
                                                              v3_x, v3_y, u3, v3, q_one,
                                                              v0_x, v0_y, u0, v0, q_one,
                                                              opacity, sub_pixel_precision,
                                                              uv_precision,
                                                              true, true, false);


    } else {

        uint8_t DIV_prec = (16 - sub_pixel_precision)>>1;
        int max = (1<<sub_pixel_precision);
        int q0 = (1<<uv_precision), q1 = (1<<uv_precision), q2 = (1<<uv_precision), q3 = (1<<uv_precision);
        int p0x = v0_x; int p0y = v0_y;
        int p1x = v1_x; int p1y = v1_y;
        int p2x = v2_x; int p2y = v2_y;
        int p3x = v3_x; int p3y = v3_y;

        int ax = p2x - p0x;
        int ay = p2y - p0y;
        int bx = p3x - p1x;
        int by = p3y - p1y;

        int t, s;

        int cross = fixed_mul_fixed_2(ax, by, sub_pixel_precision) -
                             fixed_mul_fixed_2(ay, bx, sub_pixel_precision);

        if (cross != 0) {
            int cy = p0y - p1y;
            int cx = p0x - p1x;

            int area_1 = fixed_mul_fixed_2(ax, cy, sub_pixel_precision) -
                                    fixed_mul_fixed_2(ay, cx, sub_pixel_precision);

            s = fixed_div_fixed_2((long long)area_1<<DIV_prec, cross, sub_pixel_precision);

            if (s > 0 && s < ((long long)max<<DIV_prec)) {
                int area_2 = fixed_mul_fixed_2(bx, cy, sub_pixel_precision) -
                                        fixed_mul_fixed_2(by, cx, sub_pixel_precision);

                t = fixed_div_fixed_2((long long)area_2<<DIV_prec, cross, sub_pixel_precision);

                if (t > 0 && t < ((long long)max<<DIV_prec)) {

                    q0 = fixed_div_fixed_2((long long)max<<(DIV_prec<<1), (max<<DIV_prec) - t, sub_pixel_precision);
                    q1 = fixed_div_fixed_2((long long)max<<(DIV_prec<<1), (max<<DIV_prec) - s, sub_pixel_precision);
                    q2 = fixed_div_fixed_2((long long)max<<(DIV_prec<<1), t, sub_pixel_precision);
                    q3 = fixed_div_fixed_2((long long)max<<(DIV_prec<<1), s, sub_pixel_precision);

                }
            }
        }

        int u0_q0 = fixed_mul_fixed_2(u0, q0, sub_pixel_precision)>>(DIV_prec);
        int v0_q0 = fixed_mul_fixed_2(v0, q0, sub_pixel_precision)>>(DIV_prec);
        int u1_q1 = fixed_mul_fixed_2(u1, q1, sub_pixel_precision)>>(DIV_prec);
        int v1_q1 = fixed_mul_fixed_2(v1, q1, sub_pixel_precision)>>(DIV_prec);
        int u2_q2 = fixed_mul_fixed_2(u2, q2, sub_pixel_precision)>>(DIV_prec);
        int v2_q2 = fixed_mul_fixed_2(v2, q2, sub_pixel_precision)>>(DIV_prec);
        int u3_q3 = fixed_mul_fixed_2(u3, q3, sub_pixel_precision)>>(DIV_prec);
        int v3_q3 = fixed_mul_fixed_2(v3, q3, sub_pixel_precision)>>(DIV_prec);

        q0 = fixed_convert_fixed(q0, sub_pixel_precision + DIV_prec, uv_precision);
        q1 = fixed_convert_fixed(q1, sub_pixel_precision + DIV_prec, uv_precision);
        q2 = fixed_convert_fixed(q2, sub_pixel_precision + DIV_prec, uv_precision);
        q3 = fixed_convert_fixed(q3, sub_pixel_precision + DIV_prec, uv_precision);

        // perspective correct version
        drawTriangle<BlendMode, PorterDuff, antialias, true, Sampler>(bmp,
                                                             v0_x, v0_y, u0_q0, v0_q0, q0,
                                                             v1_x, v1_y, u1_q1, v1_q1, q1,
                                                             v2_x, v2_y, u2_q2, v2_q2, q2,
                                                             opacity, sub_pixel_precision,
                                                             uv_precision,
                                                             true, true, false);

        drawTriangle<BlendMode, PorterDuff, antialias, true, Sampler>(bmp,
                                                             v2_x, v2_y, u2_q2, v2_q2, q2,
                                                             v3_x, v3_y, u3_q3, v3_q3, q3,
                                                             v0_x, v0_y, u0_q0, v0_q0, q0,
                                                             opacity, sub_pixel_precision,
                                                             uv_precision,
                                                             true, true, false);

        /*
        float q0 = 1, q1 = 1, q2 = 1, q3 = 1;

        float p0x = v0_x; float p0y = v0_y;
        float p1x = v1_x; float p1y = v1_y;
        float p2x = v2_x; float p2y = v2_y;
        float p3x = v3_x; float p3y = v3_y;

        float ax = p2x - p0x;
        float ay = p2y - p0y;
        float bx = p3x - p1x;
        float by = p3y - p1y;
        float t, s;
//    float cross = ax * by - ay * bx;
        float cross = ax * by - ay * bx;

        if (cross != 0) {
            float cy = p0y - p1y;
            float cx = p0x - p1x;

            s = float(ax * cy - ay * cx) / cross;
            if (s > 0 && s < 1) {
                t = float(bx * cy - by * cx) / cross;

                if (t > 0 && t < 1) {

                    q0 = 1 / (1 - t);
                    q1 = 1 / (1 - s);
                    q2 = 1 / t;
                    q3 = 1 / s;

                }
            }
        }

         */

    }

}

```