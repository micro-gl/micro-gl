```c++
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
         bool antialias,
         typename P2, typename CODER2>
void
canvas<P, CODER>::drawTriangle_internal(bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               const uint8_t opacity_t) {

    float area = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y});
    int bmp_width = bmp.width();
    // bounding box
    int minX = std::min({v0_x, v1_x, v2_x});
    int minY = std::min({v0_y, v1_y, v2_y});
    int maxX = std::max({v0_x, v1_x, v2_x});
    int maxY = std::max({v0_y, v1_y, v2_y});

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    unsigned int max_distance_anti_alias=0;

    if(antialias) {
        bits_distance = 1;
        max_distance_anti_alias = 1 << bits_distance;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // Triangle setup
    fixed_signed A01_u2 = float_to_fixed(u2*bmp.width()*(v0_y - v1_y)/area), B01_u2 = float_to_fixed(u2*bmp.width()*(v1_x - v0_x)/area); //w2
    fixed_signed A12_u0 = float_to_fixed(u0*bmp.width()*(v1_y - v2_y)/area), B12_u0 = float_to_fixed(u0*bmp.width()*(v2_x - v1_x)/area); // w0
    fixed_signed A20_u1 = float_to_fixed(u1*bmp.width()*(v2_y - v0_y)/area), B20_u1 = float_to_fixed(u1*bmp.width()*(v0_x - v2_x)/area); // w1

    fixed_signed A01_v2 = float_to_fixed(v2*bmp.height()*(v0_y - v1_y)/area), B01_v2 = float_to_fixed(v2*bmp.height()*(v1_x - v0_x)/area); //w2
    fixed_signed A12_v0 = float_to_fixed(v0*bmp.height()*(v1_y - v2_y)/area), B12_v0 = float_to_fixed(v0*bmp.height()*(v2_x - v1_x)/area); // w0
    fixed_signed A20_v1 = float_to_fixed(v1*bmp.height()*(v2_y - v0_y)/area), B20_v1 = float_to_fixed(v1*bmp.height()*(v0_x - v2_x)/area); // w1

//    fixed_signed A01 = v0_y - v1_y, B01 = v1_x - v0_x; //w2
//    fixed_signed A12 = v1_y - v2_y, B12 = v2_x - v1_x; // w0
//    fixed_signed A20 = v2_y - v0_y, B20 = v0_x - v2_x; // w1

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // for 16 bits computer this is safe for 7 bits input [0..127] - not good
    fixed_signed w0_row_u = float_to_fixed(float(u0*bmp.width()*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w1_row_u = float_to_fixed(float(u1*bmp.width()*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w2_row_u = float_to_fixed(float(u2*bmp.width()*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    fixed_signed w0_row_v = float_to_fixed(float(v0*bmp.height()*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w1_row_v = float_to_fixed(float(v1*bmp.height()*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w2_row_v = float_to_fixed(float(v2*bmp.height()*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    //
//    int w0_row = orient2d({v1_x, v1_y}, {v2_x, v2_y}, p);
//    int w1_row = orient2d({v2_x, v2_y}, {v0_x, v0_y}, p);
//    int w2_row = orient2d({v0_x, v0_y}, {v1_x, v1_y}, p);

    // lengths of edges
    unsigned int length_w2 = length({v0_x, v0_y}, {v1_x, v1_y});
    unsigned int length_w0 = length({v1_x, v1_y}, {v2_x, v2_y});
    unsigned int length_w1 = length({v0_x, v0_y}, {v2_x, v2_y});
//0->2, 1->0, 2->1
    // Triangle setup
    int A01 = int_to_fixed(v0_y - v1_y)/length_w2, B01 = int_to_fixed(v1_x - v0_x)/length_w2;
    int A12 = int_to_fixed(v1_y - v2_y)/length_w0, B12 = int_to_fixed(v2_x - v1_x)/length_w0;
    int A20 = int_to_fixed(v2_y - v0_y)/length_w1, B20 = int_to_fixed(v0_x - v2_x)/length_w1;

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // this is good for coordinates in the 15 bits range.
    int w2_row = ((long)int_to_fixed(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/length_w2;
    int w0_row = ((long)int_to_fixed(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/length_w0;
    int w1_row = ((long)int_to_fixed(orient2d({v2_x, v2_y}, {v0_x, v0_y}, p)))/length_w1;

    //


    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        fixed_signed w0_u = w0_row_u;
        fixed_signed w1_u = w1_row_u;
        fixed_signed w2_u = w2_row_u;

        fixed_signed w0_v = w0_row_v;
        fixed_signed w1_v = w1_row_v;
        fixed_signed w2_v = w2_row_v;

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minX; p.x <= maxX; p.x++) {
            if ((w0 | w1 | w2) >= 0) {

                int u_i = fixed_to_int((w0_u + w1_u + w2_u));//(int)(u * (float)bmp.width());
                int v_i = bmp_width * fixed_to_int((w0_v + w1_v + w2_v));//(int)(v * (float)bmp.height());
                int index_bmp = (v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity_t);

                // this is faster if we don't use blending
//                drawPixel(bmp.pixelAt(index_bmp), index + p.x);
            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + (max_distance_anti_alias<<(16));

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary

                    int u_i = clamp(fixed_to_int((w0_u + w1_u + w2_u)), 0, bmp.width() - 1);//(int)(u * (float)bmp.width());
                    int v_i = bmp_width * clamp(fixed_to_int((w0_v + w1_v + w2_v)), 0, bmp.height() - 1);//(int)(v * (float)bmp.height());
                    int index_bmp = (v_i + u_i);

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>16;

                    if (opacity_t < _max_alpha_value) {
                        blend = (blend * opacity_t) >> 8;
                    }

                    color_t col_bmp;
                    bmp.decode(index_bmp, col_bmp);
                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }

            // One step to the right
            w0_u += A12_u0;
            w1_u += A20_u1;
            w2_u += A01_u2;

            w0_v += A12_v0;
            w1_v += A20_v1;
            w2_v += A01_v2;

            w0 += A12;
            w1 += A20;
            w2 += A01;
        }

        // One row step
        w0_row_u += B12_u0;
        w1_row_u += B20_u1;
        w2_row_u += B01_u2;

        w0_row_v += B12_v0;
        w1_row_v += B20_v1;
        w2_row_v += B01_v2;

        w0_row += B12;
        w1_row += B20;
        w2_row += B01;

        index += _width;
    }

}

```

this was not faster than 2 triangles strangely
```
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
canvas<P, CODER>::drawQuadrilateral(const bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               int v3_x, int v3_y, float u3, float v3,
                               const uint8_t opacity_t) {

    float area_1 = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y})/2.0f;
    float area_2 = orient2d({v2_x, v2_y}, {v3_x, v3_y}, {v0_x, v0_y})/2.0f;
    float area = 2.0*(area_1 + area_2);

    int bmp_width = bmp.width();
    int bmp_height = bmp.height();
    // bounding box
    int minX = std::min({v0_x, v1_x, v2_x, v3_x});
    int minY = std::min({v0_y, v1_y, v2_y, v3_y});
    int maxX = std::max({v0_x, v1_x, v2_x, v3_x});
    int maxY = std::max({v0_y, v1_y, v2_y, v3_y});

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    unsigned int max_distance_anti_alias=0;

    if(antialias) {
        bits_distance = 1;
        max_distance_anti_alias = 1 << bits_distance;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // todo:: optimize all of these recurring expressions
    // Triangle setup
    fixed_signed A01_u3 = float_to_fixed(u3*bmp.width()*(v0_y - v1_y)/area), B01_u3 = float_to_fixed(u3*bmp.width()*(v1_x - v0_x)/area); //w2
    fixed_signed A12_u0 = float_to_fixed(u0*bmp.width()*(v1_y - v2_y)/area), B12_u0 = float_to_fixed(u0*bmp.width()*(v2_x - v1_x)/area); // w0
    fixed_signed A23_u1 = float_to_fixed(u1*bmp.width()*(v2_y - v3_y)/area), B23_u1 = float_to_fixed(u1*bmp.width()*(v3_x - v2_x)/area); // w1
    fixed_signed A30_u2 = float_to_fixed(u2*bmp.width()*(v3_y - v0_y)/area), B30_u2 = float_to_fixed(u2*bmp.width()*(v0_x - v3_x)/area); // w1

    fixed_signed A01_v3 = float_to_fixed(v3*bmp.height()*(v0_y - v1_y)/area), B01_v3 = float_to_fixed(v3*bmp.height()*(v1_x - v0_x)/area); //w2
    fixed_signed A12_v0 = float_to_fixed(v0*bmp.height()*(v1_y - v2_y)/area), B12_v0 = float_to_fixed(v0*bmp.height()*(v2_x - v1_x)/area); // w0
    fixed_signed A23_v1 = float_to_fixed(v1*bmp.height()*(v2_y - v3_y)/area), B23_v1 = float_to_fixed(v1*bmp.height()*(v3_x - v2_x)/area); // w1
    fixed_signed A30_v2 = float_to_fixed(v2*bmp.height()*(v3_y - v0_y)/area), B30_v2 = float_to_fixed(v2*bmp.height()*(v0_x - v3_x)/area); // w1

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };

    // 0->1, 1->2, 2->0
    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // for 16 bits computer this is safe for 7 bits input [0..127] - not good
    fixed_signed w0_row_u = float_to_fixed(float(u3*bmp.width()*(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/(area));
    fixed_signed w1_row_u = float_to_fixed(float(u0*bmp.width()*(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/(area));
    fixed_signed w2_row_u = float_to_fixed(float(u1*bmp.width()*(orient2d({v2_x, v2_y}, {v3_x, v3_y}, p)))/(area));
    fixed_signed w3_row_u = float_to_fixed(float(u2*bmp.width()*(orient2d({v3_x, v3_y}, {v0_x, v0_y}, p)))/(area));

    fixed_signed w0_row_v = float_to_fixed(float(v3*bmp.height()*(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/(area));
    fixed_signed w1_row_v = float_to_fixed(float(v0*bmp.height()*(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/(area));
    fixed_signed w2_row_v = float_to_fixed(float(v1*bmp.height()*(orient2d({v2_x, v2_y}, {v3_x, v3_y}, p)))/(area));
    fixed_signed w3_row_v = float_to_fixed(float(v2*bmp.height()*(orient2d({v3_x, v3_y}, {v0_x, v0_y}, p)))/(area));

    // lengths of edges
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y});
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y});
    unsigned int length_w2 = length({v2_x, v2_y}, {v3_x, v3_y});
    unsigned int length_w3 = length({v3_x, v3_y}, {v0_x, v0_y});

    // Triangle setup
    int A01 = int_to_fixed(v0_y - v1_y)/length_w0, B01 = int_to_fixed(v1_x - v0_x)/length_w0;
    int A12 = int_to_fixed(v1_y - v2_y)/length_w1, B12 = int_to_fixed(v2_x - v1_x)/length_w1;
    int A23 = int_to_fixed(v2_y - v3_y)/length_w2, B23 = int_to_fixed(v3_x - v2_x)/length_w2;
    int A30 = int_to_fixed(v3_y - v0_y)/length_w3, B30 = int_to_fixed(v0_x - v3_x)/length_w3;

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // this is good for coordinates in the 15 bits range.
    // this is distance to edges
    int w0_row = ((long)int_to_fixed(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/length_w0;
    int w1_row = ((long)int_to_fixed(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/length_w1;
    int w2_row = ((long)int_to_fixed(orient2d({v2_x, v2_y}, {v3_x, v3_y}, p)))/length_w2;
    int w3_row = ((long)int_to_fixed(orient2d({v3_x, v3_y}, {v0_x, v0_y}, p)))/length_w3;

    //


    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        fixed_signed w0_u = w0_row_u;
        fixed_signed w1_u = w1_row_u;
        fixed_signed w2_u = w2_row_u;
        fixed_signed w3_u = w3_row_u;

        fixed_signed w0_v = w0_row_v;
        fixed_signed w1_v = w1_row_v;
        fixed_signed w2_v = w2_row_v;
        fixed_signed w3_v = w3_row_v;

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;
        int w3 = w3_row;

        for (p.x = minX; p.x <= maxX; p.x++) {
            if ((w0 | w1 | w2 | w3) >= 0) {

//                /*
//                float ww0 = ((float)orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)/1.0f)/area;
//                float ww1 = ((float)orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)/1.0f)/area;
//                float ww2 = ((float)orient2d({v2_x, v2_y}, {v3_x, v3_y}, p)/1.0f)/area;
//                float ww3 = ((float)orient2d({v3_x, v3_y}, {v0_x, v0_y}, p)/1.0f)/area;
//
//                float u__1 =  ww0*u3 + ww1*u0 + ww2*u1 + ww3*u2;
//                float v__1 =  ww0*v3 + ww1*v0 + ww2*v1 + ww3*v2;
//
//                int u_i = (int)(u__1 * (float)bmp.width());
//                int v_i = bmp_width * (int)(v__1*(float)bmp.height());//(int)(v * (float)bmp.height());
//*/

                int u_i = fixed_to_int((w0_u + w1_u + w2_u + w3_u));//(int)(u * (float)bmp.width());
                int v_i = bmp_width * fixed_to_int((w0_v + w1_v + w2_v + w3_v));//(int)(v * (float)bmp.height());

                int index_bmp = (v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity_t);

//                drawPixel(0xFF, index + p.x);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2, w3});
                int delta = (distance) + (max_distance_anti_alias<<(16));

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary

                    int u_i = clamp(fixed_to_int((w0_u + w1_u + w2_u)), 0, bmp.width() - 1);//(int)(u * (float)bmp.width());
                    int v_i = bmp_width * clamp(fixed_to_int((w0_v + w1_v + w2_v)), 0, bmp.height() - 1);//(int)(v * (float)bmp.height());
                    int index_bmp = (v_i + u_i);

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>16;

                    if (opacity_t < _max_alpha_value) {
                        blend = (blend * opacity_t) >> 8;
                    }

                    color_t col_bmp;
                    bmp.decode(index_bmp, col_bmp);
                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }

            // One step to the right
            w0_u += A01_u3;
            w1_u += A12_u0;
            w2_u += A23_u1;
            w3_u += A30_u2;

            w0_v += A01_v3;
            w1_v += A12_v0;
            w2_v += A23_v1;
            w3_v += A30_v2;

            w0 += A01;
            w1 += A12;
            w2 += A23;
            w3 += A30;
        }

        // One row step
        w0_row_u += B01_u3;
        w1_row_u += B12_u0;
        w2_row_u += B23_u1;
        w3_row_u += B30_u2;

        w0_row_v += B01_v3;
        w1_row_v += B12_v0;
        w2_row_v += B23_v1;
        w3_row_v += B30_v2;

        w0_row += B01;
        w1_row += B12;
        w2_row += B23;
        w3_row += B30;

        index += _width;
    }

}
```

```
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename P2, typename CODER2>
void
canvas<P, CODER>::drawTriangle_internal(const bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               const uint8_t opacity_t) {
    float area = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y});
    int bmp_width = bmp.width();
    // bounding box
    int minX = std::min({v0_x, v1_x, v2_x});
    int minY = std::min({v0_y, v1_y, v2_y});
    int maxX = std::max({v0_x, v1_x, v2_x});
    int maxY = std::max({v0_y, v1_y, v2_y});

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    unsigned int max_distance_anti_alias=0;

    if(antialias) {
        bits_distance = 1;
        max_distance_anti_alias = 1 << bits_distance;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    unsigned int bmp_w_max = bmp.width() - 1, bmp_h_max = bmp.height() - 1;

    // todo:: optimize all of these recurring expressions
    // Triangle setup
    fixed_signed A01_u2 = float_to_fixed(u2*bmp_w_max*(v0_y - v1_y)/area), B01_u2 = float_to_fixed(u2*bmp_w_max*(v1_x - v0_x)/area); //w2
    fixed_signed A12_u0 = float_to_fixed(u0*bmp_w_max*(v1_y - v2_y)/area), B12_u0 = float_to_fixed(u0*bmp_w_max*(v2_x - v1_x)/area); // w0
    fixed_signed A20_u1 = float_to_fixed(u1*bmp_w_max*(v2_y - v0_y)/area), B20_u1 = float_to_fixed(u1*bmp_w_max*(v0_x - v2_x)/area); // w1

    fixed_signed A01_v2 = float_to_fixed(v2*bmp_h_max*(v0_y - v1_y)/area), B01_v2 = float_to_fixed(v2*bmp_h_max*(v1_x - v0_x)/area); //w2
    fixed_signed A12_v0 = float_to_fixed(v0*bmp_h_max*(v1_y - v2_y)/area), B12_v0 = float_to_fixed(v0*bmp_h_max*(v2_x - v1_x)/area); // w0
    fixed_signed A20_v1 = float_to_fixed(v1*bmp_h_max*(v2_y - v0_y)/area), B20_v1 = float_to_fixed(v1*bmp_h_max*(v0_x - v2_x)/area); // w1

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };

    // 0->1, 1->2, 2->0
    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // for 16 bits computer this is safe for 7 bits input [0..127] - not good
    fixed_signed w1_row_u = float_to_fixed(float(u0*bmp_w_max*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w2_row_u = float_to_fixed(float(u1*bmp_w_max*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w0_row_u = float_to_fixed(float(u2*bmp_w_max*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    fixed_signed w1_row_v = float_to_fixed(float(v0*bmp_h_max*orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area);
    fixed_signed w2_row_v = float_to_fixed(float(v1*bmp_h_max*orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area);
    fixed_signed w0_row_v = float_to_fixed(float(v2*bmp_h_max*orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area);

    // lengths of edges
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y});
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y});
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y});
//0->2, 1->0, 2->1
    // Triangle setup
    int A01 = int_to_fixed(v0_y - v1_y)/length_w0, B01 = int_to_fixed(v1_x - v0_x)/length_w0;
    int A12 = int_to_fixed(v1_y - v2_y)/length_w1, B12 = int_to_fixed(v2_x - v1_x)/length_w1;
    int A20 = int_to_fixed(v2_y - v0_y)/length_w2, B20 = int_to_fixed(v0_x - v2_x)/length_w2;

    // overflow safety safe_bits>=(p-2)/2, i.e 15 bits (0..32,768) for 32 bits integers.
    // https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
    // this is good for coordinates in the 15 bits range.
    // this is distance to edges
    int w0_row = ((long)int_to_fixed(orient2d({v0_x, v0_y}, {v1_x, v1_y}, p)))/length_w0;
    int w1_row = ((long)int_to_fixed(orient2d({v1_x, v1_y}, {v2_x, v2_y}, p)))/length_w1;
    int w2_row = ((long)int_to_fixed(orient2d({v2_x, v2_y}, {v0_x, v0_y}, p)))/length_w2;

    //


    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        fixed_signed w0_u = w0_row_u;
        fixed_signed w1_u = w1_row_u;
        fixed_signed w2_u = w2_row_u;

        fixed_signed w0_v = w0_row_v;
        fixed_signed w1_v = w1_row_v;
        fixed_signed w2_v = w2_row_v;

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minX; p.x <= maxX; p.x++) {
            if ((w0 | w1 | w2) >= 0) {

                //                /*
                float ww0 = ((float)orient2d({v0_x, v0_y}, {v1_x, v1_y}, p))/area;
                float ww1 = ((float)orient2d({v1_x, v1_y}, {v2_x, v2_y}, p))/area;
                float ww2 = ((float)orient2d({v2_x, v2_y}, {v0_x, v0_y}, p))/area;

                float uuu =  ww0*u2 + ww1*u0 + ww2*u1;
                float vvv =  ww0*v2 + ww1*v0 + ww2*v1;

                unsigned int u_i = bmp_w_max*uuu;
                unsigned int v_i = bmp_h_max*vvv;
                int index_bmp = (bmp_width*v_i + u_i);

//                int u_i = (int)(u__1 * (float)(bmp.width()-1));
//                int v_i = bmp_width * (int)(v__1*(float)(bmp.height()-1));//(int)(v * (float)bmp.height());
//

/*
                int u_i = fixed_to_int((w0_u + w1_u + w2_u));
                int v_i = fixed_to_int((w0_v + w1_v + w2_v));
                // i should clamp if i see artifacts
//                int u_i = clamp(fixed_to_int((w0_u + w1_u + w2_u)), 0, bmp_w_max);
//                int v_i = clamp(fixed_to_int((w0_v + w1_v + w2_v)), 0, bmp_h_max);
                int index_bmp = (bmp_width*v_i + u_i);
                */

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity_t);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + (max_distance_anti_alias<<(16));

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary

                    int u_i = clamp(fixed_to_int((w0_u + w1_u + w2_u)), 0, bmp_w_max);
                    int v_i = clamp(fixed_to_int((w0_v + w1_v + w2_v)), 0, bmp_h_max);
                    int index_bmp = bmp_width *v_i + u_i;

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>16;

                    if (opacity_t < _max_alpha_value) {
                        blend = (blend * opacity_t) >> 8;
                    }

                    color_t col_bmp;
                    bmp.decode(index_bmp, col_bmp);
                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }

            // One step to the right
            w0_u += A01_u2;
            w1_u += A12_u0;
            w2_u += A20_u1;

            w0_v += A01_v2;
            w1_v += A12_v0;
            w2_v += A20_v1;

            w0 += A01;
            w1 += A12;
            w2 += A20;
        }

        // One row step
        w0_row_u += B01_u2;
        w1_row_u += B12_u0;
        w2_row_u += B20_u1;

        w0_row_v += B01_v2;
        w1_row_v += B12_v0;
        w2_row_v += B20_v1;

        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        index += _width;
    }

}

```


```
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, bool perspective_correct,
        typename P2, typename CODER2>
void
canvas<P, CODER>::drawTriangle_internal(const bitmap<P2, CODER2> & bmp,
                               const fixed_signed v0_x, const fixed_signed v0_y, float u0, float v0, float q0,
                               const fixed_signed v1_x, const fixed_signed v1_y, float u1, float v1, float q1,
                               const fixed_signed v2_x, const fixed_signed v2_y, float u2, float v2, float q2,
                               const uint8_t opacity_t, const uint8_t sub_pixel_precision) {

    fixed_signed area = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    int bmp_width = bmp.width();

    // sub_pixel_precision;
    // THIS MAY HAVE TO BE MORE LIKE 15 TO AVOID OVERFLOW
    uint8_t BITS_COORDS_INTEGRAL = 10 + 1;
    uint8_t BITS_COORDS_FRAC = sub_pixel_precision;
    uint8_t BITS_BITMAP_W = 8;
    uint8_t BITS_BITMAP_H = 8;
    uint8_t BITS_UV_COORDS = 5;

    uint8_t MAX_BITS_FOR_PROCESSING_PRECISION = 32;
//    uint8_t PR = MAX_BITS_FOR_PROCESSING_PRECISION - (BITS_COORDS_INTEGRAL + BITS_COORDS_FRAC + BITS_BITMAP_W + BITS_UV_COORDS);
    uint8_t PR = 16 - (BITS_COORDS_FRAC);
    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    // bounding box
    int minX = (std::min({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int minY = (std::min({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxX = (std::max({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxY = (std::max({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    // max distance to consider in canvas space
    unsigned int max_distance_canvas_space_anti_alias=0;
    // max distance to consider in scaled space
    unsigned int max_distance_scaled_space_anti_alias=0;

    if(antialias) {
        bits_distance = 0;
        max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias<<PR;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
//        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
//        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

//    minX=maxX=450;
//    minY=maxY=200;
//
//    minX=-0;
//    minY=0;

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };
    vec2_fixed_signed p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };

    int bmp_w_max = bmp.width() - 1, bmp_h_max = bmp.height() - 1;

    // convert the floats to fixed point integers
    fixed_signed u0_fixed = float_to_fixed_2(u0, PR), v0_fixed = float_to_fixed_2(v0, PR);
    fixed_signed u1_fixed = float_to_fixed_2(u1, PR), v1_fixed = float_to_fixed_2(v1, PR);
    fixed_signed u2_fixed = float_to_fixed_2(u2, PR), v2_fixed = float_to_fixed_2(v2, PR);
    fixed_signed q0_fixed, q1_fixed, q2_fixed;
    uint8_t Q_bits= 18;
    if(perspective_correct) {
        q0_fixed = float_to_fixed_2(q0, Q_bits), q1_fixed = float_to_fixed_2(q1, Q_bits), q2_fixed = float_to_fixed_2(q2, Q_bits);
    }

    fixed_signed A01_u2 = fixed_mul_int(u2_fixed, bmp_w_max*(v0_y - v1_y))/area, B01_u2 = fixed_mul_int(u2_fixed, bmp_w_max*(v1_x - v0_x))/area; // w0
    fixed_signed A12_u0 = fixed_mul_int(u0_fixed, bmp_w_max*(v1_y - v2_y))/area, B12_u0 = fixed_mul_int(u0_fixed, bmp_w_max*(v2_x - v1_x))/area; // w1
    fixed_signed A20_u1 = fixed_mul_int(u1_fixed, bmp_w_max*(v2_y - v0_y))/area, B20_u1 = fixed_mul_int(u1_fixed, bmp_w_max*(v0_x - v2_x))/area; // w2

    fixed_signed A01_v2 = fixed_mul_int(v2_fixed, bmp_h_max*(v0_y - v1_y))/area, B01_v2 = fixed_mul_int(v2_fixed, bmp_h_max*(v1_x - v0_x))/area; // w0
    fixed_signed A12_v0 = fixed_mul_int(v0_fixed, bmp_h_max*(v1_y - v2_y))/area, B12_v0 = fixed_mul_int(v0_fixed, bmp_h_max*(v2_x - v1_x))/area; // w1
    fixed_signed A20_v1 = fixed_mul_int(v1_fixed, bmp_h_max*(v2_y - v0_y))/area, B20_v1 = fixed_mul_int(v1_fixed, bmp_h_max*(v0_x - v2_x))/area; // w2

    fixed_signed A01_q2, A12_q0, A20_q1, B01_q2, B12_q0, B20_q1;
    if(perspective_correct) {
        A01_q2 = fixed_mul_int(q2_fixed, (v0_y - v1_y))/area, B01_q2 = fixed_mul_int(q2_fixed, (v1_x - v0_x))/area; // w0
        A12_q0 = fixed_mul_int(q0_fixed, (v1_y - v2_y))/area, B12_q0 = fixed_mul_int(q0_fixed, (v2_x - v1_x))/area; // w1
        A20_q1 = fixed_mul_int(q1_fixed, (v2_y - v0_y))/area, B20_q1 = fixed_mul_int(q1_fixed, (v0_x - v2_x))/area; // w2
    }

    // this can produce a 2P bits number if the points form a a perpendicular triangle
    fixed_signed area_v1_v2_p = orient2d({v1_x, v1_y}, {v2_x, v2_y}, p_fixed, sub_pixel_precision),
                area_v2_v0_p = orient2d({v2_x, v2_y}, {v0_x, v0_y}, p_fixed, sub_pixel_precision),
                area_v0_v1_p = orient2d({v0_x, v0_y}, {v1_x, v1_y}, p_fixed, sub_pixel_precision);

    fixed_signed w1_row_u = fixed_mul_int(u0_fixed, bmp_w_max*area_v1_v2_p)/area;
    fixed_signed w2_row_u = fixed_mul_int(u1_fixed, bmp_w_max*area_v2_v0_p)/area;
    fixed_signed w0_row_u = fixed_mul_int(u2_fixed, bmp_w_max*area_v0_v1_p)/area;

    fixed_signed w1_row_v = fixed_mul_int(v0_fixed, bmp_h_max*area_v1_v2_p)/area;
    fixed_signed w2_row_v = fixed_mul_int(v1_fixed, bmp_h_max*area_v2_v0_p)/area;
    fixed_signed w0_row_v = fixed_mul_int(v2_fixed, bmp_h_max*area_v0_v1_p)/area;

    fixed_signed w1_row_q, w2_row_q, w0_row_q;

    if(perspective_correct) {
        w1_row_q = fixed_mul_int(q0_fixed, area_v1_v2_p) / area;
        w2_row_q = fixed_mul_int(q1_fixed, area_v2_v0_p) / area;
        w0_row_q = fixed_mul_int(q2_fixed, area_v0_v1_p) / area;
    }

    // lengths of edges, produces a P+1 bits number
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y}, sub_pixel_precision);
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y}, sub_pixel_precision);


    // PR seems very good for the following calculations
    // Triangle setup
    // this needs at least (P+1) bits, since the delta is always <= length
    int A01 = int_to_fixed_2(v0_y - v1_y, PR)/length_w0, B01 = int_to_fixed_2(v1_x - v0_x, PR)/length_w0;
    int A12 = int_to_fixed_2(v1_y - v2_y, PR)/length_w1, B12 = int_to_fixed_2(v2_x - v1_x, PR)/length_w1;
    int A20 = int_to_fixed_2(v2_y - v0_y, PR)/length_w2, B20 = int_to_fixed_2(v0_x - v2_x, PR)/length_w2;

    // 2A/L = h, therefore the division produces a P bit number
    // we should use around 31-MAX_BITS_PRES
    int w0_row = int_to_fixed_2(area_v0_v1_p, PR)/length_w0;
    int w1_row = int_to_fixed_2(area_v1_v2_p, PR)/length_w1;
    int w2_row = int_to_fixed_2(area_v2_v0_p, PR)/length_w2;
//    PR=12;
//floor(((3200<<16)/7368))/2^16
    fixed_signed half = (1<<(PR))>>1;
    long one_extended = (((long)1)<<((PR)<<1));

    /*
    // LUT experiment
    static fixed_signed dic[(2<<17)+1];
    static bool filled=false;
    if(!filled) {
        for (int ix = 1; ix < (1<<17)+1; ++ix) {
            dic[ix] = (fixed_signed) fixed_one_over_fixed_2(ix, PR);
        }
        filled=true;
        return;
    }
     */

    //

    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        fixed_signed w0_u = w0_row_u;
        fixed_signed w1_u = w1_row_u;
        fixed_signed w2_u = w2_row_u;

        fixed_signed w0_v = w0_row_v;
        fixed_signed w1_v = w1_row_v;
        fixed_signed w2_v = w2_row_v;

        fixed_signed w0_q, w1_q, w2_q;

        if(perspective_correct) {
            w0_q = w0_row_q;
            w1_q = w1_row_q;
            w2_q = w2_row_q;
        }

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minX; p.x <= maxX; p.x++) {
            float ww0 = ((float)orient2d({v0_x, v0_y}, {v1_x, v1_y}, {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))/area;
            float ww1 = ((float)orient2d({v1_x, v1_y}, {v2_x, v2_y}, {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))/area;
            float ww2 = ((float)orient2d({v2_x, v2_y}, {v0_x, v0_y},  {p.x<<sub_pixel_precision, p.y<<sub_pixel_precision},sub_pixel_precision))/area;

//            if (ww0>=0 && ww1>=0 &&ww2>=0) {

            if ((w0 | w1 | w2) >= 0) {

                //                /*

                float u__1 =  ww0*u2 + ww1*u0 + ww2*u1;
                float v__1 =  ww0*v2 + ww1*v0 + ww2*v1;

//*/

                fixed_signed u_fixed = w0_u + w1_u + w2_u + half;
                fixed_signed v_fixed = w0_v + w1_v + w2_v + half;

                if(perspective_correct) {
                    // this optimizes fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR),
                    // I basically save two shifts
                    fixed_signed q_fixed = (w0_q + w1_q + w2_q)>>(Q_bits - PR);
                    fixed_signed qqs = one_extended/q_fixed;
//                    fixed_signed qqs = (w0_q + w1_q + w2_q-1)==0?0:fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR);//one_extended/(w0_q + w1_q + w2_q);
//                    fixed_signed qqs = dic[(w0_q + w1_q + w2_q)];
                    u_fixed = fixed_div_fixed_2(u_fixed, q_fixed, PR);
                    v_fixed = fixed_div_fixed_2(v_fixed, q_fixed, PR);

//                    u_fixed = fixed_mul_fixed_2(u_fixed, qqs, PR);
//                    v_fixed = fixed_mul_fixed_2(v_fixed, qqs, PR);
                }

                // we round the math, which greatly improves things
                 int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
                 int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                int index_bmp = (bmp_width*v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity_t);

//                drawPixel(0xFF, index+p.x);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + max_distance_scaled_space_anti_alias;

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary
                    // I don't round since I don't care about it here

                    fixed_signed u_fixed = w0_u + w1_u + w2_u + half;
                    fixed_signed v_fixed = w0_v + w1_v + w2_v + half;

                    if(perspective_correct) {
                        // this optimizes fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR),
                        // I basically save two shifts
                        fixed_signed qqs = one_extended/(w0_q + w1_q + w2_q);

                        u_fixed = fixed_mul_fixed_2(u_fixed, qqs, PR);
                        v_fixed = fixed_mul_fixed_2(v_fixed, qqs, PR);
                    }

                    int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
                    int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                    int index_bmp = bmp_width *v_i + u_i;

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>PR;

                    if (opacity_t < _max_alpha_value) {
                        blend = (blend * opacity_t) >> 8;
                    }

                    color_t col_bmp;
                    bmp.decode(index_bmp, col_bmp);
                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }

            // One step to the right
            w0_u += A01_u2;
            w1_u += A12_u0;
            w2_u += A20_u1;

            w0_v += A01_v2;
            w1_v += A12_v0;
            w2_v += A20_v1;

            if(perspective_correct) {
                w0_q += A01_q2;
                w1_q += A12_q0;
                w2_q += A20_q1;
            }

            w0 += A01;
            w1 += A12;
            w2 += A20;
        }

        // One row step
        w0_row_u += B01_u2;
        w1_row_u += B12_u0;
        w2_row_u += B20_u1;

        w0_row_v += B01_v2;
        w1_row_v += B12_v0;
        w2_row_v += B20_v1;

        if(perspective_correct) {
            w0_row_q += B01_q2;
            w1_row_q += B12_q0;
            w2_row_q += B20_q1;
        }

        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        index += _width;
    }

}
```


Older circle without sub-pixel correction 
```
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void canvas<P, CODER>::drawCircle(const color_f_t & color,
                                  int centerX, int centerY,
                                  int radius,
                                  uint8_t opacity_t) {
    color_t color_int;

    coder()->convert(color, color_int);

    unsigned int bits_for_antialias_distance, max_blend_distance=0;
    unsigned int a, b, c=0;

    if(antialias) {
        bits_for_antialias_distance = 1;
        max_blend_distance = 1 << bits_for_antialias_distance;
        a = radius*radius;
        b = (radius+max_blend_distance)*(radius+max_blend_distance);
        c = b - a;
    }

    bool apply_opacity = opacity_t!=255;
    int delta;

    int x_min = centerX - radius - max_blend_distance, y_min = centerY - radius - max_blend_distance;
    int x_max = centerX + radius + max_blend_distance, y_max = centerY + radius + max_blend_distance;
    x_min = std::max(0, x_min); y_min = std::max(0, y_min);
    x_max = std::min(width(), x_max); y_max = std::min(height(), y_max);

    int index;

    for (int y = y_min; y < y_max; ++y) {
        index = y * _width;
        for (int x = x_min; x < x_max; ++x) {

            // 16 bit precision fixed point
            int distance = signed_distance_circle_raised_quad(x, y, centerX, centerY, radius);

            if(distance<=0)
                blendColor<BlendMode, PorterDuff>(color_int, index + x, opacity_t);
            else if(antialias && (delta=c-distance)>=0){

//                 scale inner to 8 bit and then convert to integer
                uint8_t blend = ((delta)<<(8))/c;

                if(apply_opacity)
                    blend = (blend*opacity_t)>>8;

                blendColor<BlendMode, PorterDuff>(color_int, index + x, blend);
            }

        }

    }

}

```

Old antialias for circle I found
```
template<typename P, typename CODER>
void canvas<P, CODER>::drawCircle(const color_f_t & color,
                           int centerX, int centerY,
                           int radius) {
    uint8_t nSubpixelsX ,nSubpixelsY;
    color_f_t color_res = color;

    nSubpixelsX = nSubpixelsY = 3;//hasAntialiasing() ? 4 : 1;

    int x1 = centerX - radius, y1 = centerY - radius;
    int x2 = centerX + radius, y2 = centerY + radius;
    int index;

    for (int y = y1; y < y2; ++y) {
        // this is an optimization instead of multiplying per pixel
        index = y * _width;
        for (int x = x1; x < x2; ++x) {

            // Compute the coverage by sampling the circle at "subpixel"
            // locations and counting the number of subpixels turned on.
            float coverage = 0.0f;

            for (int subpixelY = 0; subpixelY < nSubpixelsY; subpixelY++) {
                for (int subpixelX = 0; subpixelX < nSubpixelsX; subpixelX++) {
                    // Sample the center of the subpixel.
                    float sampX = x + ((subpixelX + 0.5f) / nSubpixelsX);
                    float sampY = y + ((subpixelY + 0.5f) / nSubpixelsY);
                    if (insideCircle(sampX, sampY, centerX, centerY, radius))
                        coverage += 1;
                }
            }

            // Take the average of all subpixels.
            coverage /= nSubpixelsX * nSubpixelsY;

            // Quick optimization: if we're fully outside the circle,
            // we don't need to compute the fill.
            if (coverage == 0)
                continue;

            color_res.a = color.a * coverage;
            blendColor(color_res, index + x);

        }

    }

}

```

lines

```
template<typename P, typename CODER>
void canvas<P, CODER>::drawWuLine_internal(const color_f_t &color, int x0, int y0, int x1, int y1) {
    int X0 = x0, Y0 = y0, X1 = x1, Y1=y1;
    color_t color_input{};

    coder()->convert(color, color_input);

    unsigned int IntensityBits = 8;
    unsigned int NumLevels = 1 << IntensityBits;
    unsigned int maxIntensity = NumLevels - 1;
    unsigned int IntensityShift, ErrorAdj, ErrorAcc;
    unsigned int ErrorAccTemp, Weighting, WeightingComplementMask;
    int DeltaX, DeltaY, Temp, XDir;

    // Make sure the line runs top to bottom
    if (Y0 > Y1) {
        Temp = Y0; Y0 = Y1; Y1 = Temp;
        Temp = X0; X0 = X1; X1 = Temp;
    }

    // Draw the initial pixel, which is always exactly intersected by
    // the line and so needs no weighting
    blendColor(color_input, X0, Y0, maxIntensity);

    if ((DeltaX = X1 - X0) >= 0) {
        XDir = 1;
    } else {
        XDir = -1;
        DeltaX = -DeltaX; // make DeltaX positive
    }
    DeltaY = Y1 - Y0;

    // Special-case horizontal, vertical, and diagonal lines, which
    // require no weighting because they go right through the center of
    // every pixel
    if ((Y1 - Y0) == 0) {
        // Horizontal line
        while (DeltaX-- != 0) {
            X0 += XDir;
            blendColor(color_input, X0, Y0, maxIntensity);

        }
        return;
    }
    if (DeltaX == 0) {
        // Vertical line
        do {
            Y0++;
            blendColor(color_input, X0, Y0, maxIntensity);
        } while (--DeltaY != 0);
        return;
    }
    if (DeltaX == DeltaY) {
        // Diagonal line
        do {
            X0 += XDir;
            Y0++;
            blendColor(color_input, X0, Y0, maxIntensity);
        } while (--DeltaY != 0);
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
        while (--DeltaY) {
            ErrorAccTemp = ErrorAcc; // remember currrent accumulated error
            ErrorAcc += ErrorAdj; // calculate error for next pixel
            if (ErrorAcc <= ErrorAccTemp) {
                // The error accumulator turned over, so advance the X coord
                X0 += XDir;
            }
            Y0++; // Y-major, so always advance Y
            // The IntensityBits most significant bits of ErrorAcc give us the
            // intensity weighting for this pixel, and the complement of the
            // weighting for the paired pixel
            Weighting = ErrorAcc >> IntensityShift;

            unsigned int mix = (Weighting ^ WeightingComplementMask);
            // this equals Weighting, but I write it like that for clarity for now
            unsigned int mix_complement = maxIntensity - mix;

            blendColor(color_input, X0, Y0, mix);
            blendColor(color_input, X0 + XDir, Y0, mix_complement);
        }

        // Draw the final pixel, which is always exactly intersected by the line
        // and so needs no weighting
        blendColor(color_input, X1, Y1, maxIntensity);
        return;
    }

    // It's an X-major line; calculate 16-bit fixed-point fractional part of a
    // pixel that Y advances each time X advances 1 pixel, truncating the
    // result to avoid overrunning the endpoint along the X axis
    ErrorAdj = ((unsigned long long) DeltaY << 32) / (unsigned long long) DeltaX;

    // Draw all pixels other than the first and last
    while (--DeltaX) {
        ErrorAccTemp = ErrorAcc; // remember currrent accumulated error
        ErrorAcc += ErrorAdj; // calculate error for next pixel
        if (ErrorAcc <= ErrorAccTemp) {
            // The error accumulator turned over, so advance the Y coord
            Y0++;
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

        blendColor(color_input, X0, Y0, mix);
        blendColor(color_input, X0, Y0 + 1, mix_complement);
    }

    // Draw the final pixel, which is always exactly intersected by the line
    // and so needs no weighting
    blendColor(color_input, X1, Y1, maxIntensity);
}

```
```
/*
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, bool perspective_correct,
        typename Sampler,
        typename P2, typename CODER2>
void
canvas<P, CODER>::drawTriangle2(const bitmap<P2, CODER2> & bmp,
                               const fixed_signed v0_x, const fixed_signed v0_y, fixed_signed u0, fixed_signed v0, fixed_signed q0,
                               const fixed_signed v1_x, const fixed_signed v1_y, fixed_signed u1, fixed_signed v1, fixed_signed q1,
                               const fixed_signed v2_x, const fixed_signed v2_y, fixed_signed u2, fixed_signed v2, fixed_signed q2,
                               const uint8_t opacity_t, const uint8_t sub_pixel_precision, const uint8_t uv_precision) {

    fixed_signed area = orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    int bmp_width = bmp.width();

    // sub_pixel_precision;
    // THIS MAY HAVE TO BE MORE LIKE 15 TO AVOID OVERFLOW
    uint8_t BITS_COORDS_INTEGRAL = 10 + 1;
    uint8_t BITS_COORDS_FRAC = sub_pixel_precision;
    uint8_t BITS_BITMAP_W = 8;
    uint8_t BITS_BITMAP_H = 8;
    uint8_t BITS_UV_COORDS = uv_precision;

    unsigned int max_sub_pixel_precision_value = (1<<sub_pixel_precision) - 1;

    uint8_t BITS_DIV_PREC = 8;
    uint8_t BITS_Q_DIV_PREC = 20 - BITS_UV_COORDS;
    uint8_t PR = BITS_DIV_PREC + BITS_UV_COORDS;
    uint8_t PR_Q = BITS_Q_DIV_PREC + BITS_UV_COORDS;

    // bounding box
    int minX = (std::min({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int minY = (std::min({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxX = (std::max({v0_x, v1_x, v2_x}) + max_sub_pixel_precision_value) >> sub_pixel_precision;
    int maxY = (std::max({v0_y, v1_y, v2_y}) + max_sub_pixel_precision_value) >> sub_pixel_precision;

    // anti-alias pad for distance calculation
    uint8_t bits_distance;
    // max distance to consider in canvas space
    unsigned int max_distance_canvas_space_anti_alias=0;
    // max distance to consider in scaled space
    unsigned int max_distance_scaled_space_anti_alias=0;

    if(antialias) {
        bits_distance = 0;
        max_distance_canvas_space_anti_alias = 1 << bits_distance;
        max_distance_scaled_space_anti_alias = max_distance_canvas_space_anti_alias<<PR;
        // we can solve padding analytically with distance=(max_distance_anti_alias/Cos(angle))
        // but I don't give a fuck about it since I am just using max value of 2
//        minX-=max_distance_anti_alias*2;minY-=max_distance_anti_alias*2;
//        maxX+=max_distance_anti_alias*2;maxY+=max_distance_anti_alias*2;
    }

    // clipping
    minX = std::max(0, minX); minY = std::max(0, minY);
    maxX = std::min(width()-1, maxX); maxY = std::min(height()-1, maxY);

    // Barycentric coordinates at minX/minY corner
    vec2_32i p = { minX, minY };
    vec2_fixed_signed p_fixed = { minX<<sub_pixel_precision, minY<<sub_pixel_precision };

    int bmp_w_max = bmp.width() - 1, bmp_h_max = bmp.height() - 1;

    // convert the floats to fixed point integers
    fixed_signed u0_fixed = (u0), v0_fixed = (v0);
    fixed_signed u1_fixed = (u1), v1_fixed = (v1);
    fixed_signed u2_fixed = (u2), v2_fixed = (v2);
    fixed_signed q0_fixed, q1_fixed, q2_fixed;
    uint8_t Q_bits= PR;
    if(perspective_correct) {
        q0_fixed = (q0), q1_fixed = (q1), q2_fixed = (q2);
    }

    fixed_signed A01_u2 = fixed_mul_int(u2_fixed, (long)bmp_w_max*((v0_y - v1_y)<<BITS_DIV_PREC))/area, B01_u2 = fixed_mul_int(u2_fixed, (long)bmp_w_max*((v1_x - v0_x)<<BITS_DIV_PREC))/area; // w0
    fixed_signed A12_u0 = fixed_mul_int(u0_fixed, (long)bmp_w_max*((v1_y - v2_y)<<BITS_DIV_PREC))/area, B12_u0 = fixed_mul_int(u0_fixed, (long)bmp_w_max*((v2_x - v1_x)<<BITS_DIV_PREC))/area; // w1
    fixed_signed A20_u1 = fixed_mul_int(u1_fixed, (long)bmp_w_max*((v2_y - v0_y)<<BITS_DIV_PREC))/area, B20_u1 = fixed_mul_int(u1_fixed, (long)bmp_w_max*((v0_x - v2_x)<<BITS_DIV_PREC))/area; // w2

    fixed_signed A01_v2 = fixed_mul_int(v2_fixed, (long)bmp_h_max*((v0_y - v1_y)<<BITS_DIV_PREC))/area, B01_v2 = fixed_mul_int(v2_fixed, (long)bmp_h_max*((v1_x - v0_x)<<BITS_DIV_PREC))/area; // w0
    fixed_signed A12_v0 = fixed_mul_int(v0_fixed, (long)bmp_h_max*((v1_y - v2_y)<<BITS_DIV_PREC))/area, B12_v0 = fixed_mul_int(v0_fixed, (long)bmp_h_max*((v2_x - v1_x)<<BITS_DIV_PREC))/area; // w1
    fixed_signed A20_v1 = fixed_mul_int(v1_fixed, (long)bmp_h_max*((v2_y - v0_y)<<BITS_DIV_PREC))/area, B20_v1 = fixed_mul_int(v1_fixed, (long)bmp_h_max*((v0_x - v2_x)<<BITS_DIV_PREC))/area; // w2

    fixed_signed A01_q2, A12_q0, A20_q1, B01_q2, B12_q0, B20_q1;
    if(perspective_correct) {
        A01_q2 = fixed_mul_int(q2_fixed, (long)(v0_y - v1_y)<<BITS_Q_DIV_PREC)/area, B01_q2 = fixed_mul_int(q2_fixed, (long)(v1_x - v0_x)<<BITS_Q_DIV_PREC)/area; // w0
        A12_q0 = fixed_mul_int(q0_fixed, (long)(v1_y - v2_y)<<BITS_Q_DIV_PREC)/area, B12_q0 = fixed_mul_int(q0_fixed, (long)(v2_x - v1_x)<<BITS_Q_DIV_PREC)/area; // w1
        A20_q1 = fixed_mul_int(q1_fixed, (long)(v2_y - v0_y)<<BITS_Q_DIV_PREC)/area, B20_q1 = fixed_mul_int(q1_fixed, (long)(v0_x - v2_x)<<BITS_Q_DIV_PREC)/area; // w2
    }

    // this can produce a 2P bits number if the points form a a perpendicular triangle
    fixed_signed area_v1_v2_p = orient2d({v1_x, v1_y}, {v2_x, v2_y}, p_fixed, sub_pixel_precision),
            area_v2_v0_p = orient2d({v2_x, v2_y}, {v0_x, v0_y}, p_fixed, sub_pixel_precision),
            area_v0_v1_p = orient2d({v0_x, v0_y}, {v1_x, v1_y}, p_fixed, sub_pixel_precision);

    fixed_signed w1_row_u = fixed_mul_int(u0_fixed, (long)bmp_w_max*(area_v1_v2_p<<BITS_DIV_PREC))/area;
    fixed_signed w2_row_u = fixed_mul_int(u1_fixed, (long)bmp_w_max*(area_v2_v0_p<<BITS_DIV_PREC))/area;
    fixed_signed w0_row_u = fixed_mul_int(u2_fixed, (long)bmp_w_max*(area_v0_v1_p<<BITS_DIV_PREC))/area;

    fixed_signed w1_row_v = fixed_mul_int(v0_fixed, (long)bmp_h_max*(area_v1_v2_p<<BITS_DIV_PREC))/area;
    fixed_signed w2_row_v = fixed_mul_int(v1_fixed, (long)bmp_h_max*(area_v2_v0_p<<BITS_DIV_PREC))/area;
    fixed_signed w0_row_v = fixed_mul_int(v2_fixed, (long)bmp_h_max*(area_v0_v1_p<<BITS_DIV_PREC))/area;

    fixed_signed w1_row_q, w2_row_q, w0_row_q;

    if(perspective_correct) {
        w1_row_q = fixed_mul_int(q0_fixed, ((long)area_v1_v2_p)<<BITS_Q_DIV_PREC) / area;
        w2_row_q = fixed_mul_int(q1_fixed, ((long)area_v2_v0_p)<<BITS_Q_DIV_PREC) / area;
        w0_row_q = fixed_mul_int(q2_fixed, ((long)area_v0_v1_p)<<BITS_Q_DIV_PREC) / area;
    }

    // lengths of edges, produces a P+1 bits number
    unsigned int length_w0 = length({v0_x, v0_y}, {v1_x, v1_y}, sub_pixel_precision);
    unsigned int length_w1 = length({v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    unsigned int length_w2 = length({v0_x, v0_y}, {v2_x, v2_y}, sub_pixel_precision);


    // PR seems very good for the following calculations
    // Triangle setup
    // this needs at least (P+1) bits, since the delta is always <= length
    int A01 = int_to_fixed_2(v0_y - v1_y, PR)/length_w0, B01 = int_to_fixed_2(v1_x - v0_x, PR)/length_w0;
    int A12 = int_to_fixed_2(v1_y - v2_y, PR)/length_w1, B12 = int_to_fixed_2(v2_x - v1_x, PR)/length_w1;
    int A20 = int_to_fixed_2(v2_y - v0_y, PR)/length_w2, B20 = int_to_fixed_2(v0_x - v2_x, PR)/length_w2;

    // 2A/L = h, therefore the division produces a P bit number
    // we should use around 31-MAX_BITS_PRES
    int w0_row = int_to_fixed_2(area_v0_v1_p, PR)/length_w0;
    int w1_row = int_to_fixed_2(area_v1_v2_p, PR)/length_w1;
    int w2_row = int_to_fixed_2(area_v2_v0_p, PR)/length_w2;
    fixed_signed half = (1<<(PR))>>1;
    long one_extended = (((long)1)<<((PR)<<1));

    //

    int index = p.y * _width;

    for (p.y = minY; p.y <= maxY; p.y++) {

        // Barycentric coordinates at start of row
        fixed_signed w0_u = w0_row_u;
        fixed_signed w1_u = w1_row_u;
        fixed_signed w2_u = w2_row_u;

        fixed_signed w0_v = w0_row_v;
        fixed_signed w1_v = w1_row_v;
        fixed_signed w2_v = w2_row_v;

        fixed_signed w0_q, w1_q, w2_q;

        if(perspective_correct) {
            w0_q = w0_row_q;
            w1_q = w1_row_q;
            w2_q = w2_row_q;
        }

        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (p.x = minX; p.x <= maxX; p.x++) {

            if ((w0 | w1 | w2) >= 0) {

                fixed_signed u_fixed = w0_u + w1_u + w2_u + half;
                fixed_signed v_fixed = w0_v + w1_v + w2_v + half;

                if(perspective_correct) {
                    // this optimizes fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR),
                    // I basically save two shifts
                    fixed_signed q_fixed = (w0_q + w1_q + w2_q)>>(-PR+PR_Q);
                    fixed_signed qqs = one_extended/q_fixed;
//                    fixed_signed qqs = (w0_q + w1_q + w2_q-1)==0?0:fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR);//one_extended/(w0_q + w1_q + w2_q);
//                    fixed_signed qqs = dic[(w0_q + w1_q + w2_q)];
                    u_fixed = fixed_div_fixed_2(u_fixed, q_fixed, PR);
                    v_fixed = fixed_div_fixed_2(v_fixed, q_fixed, PR);

//                    u_fixed = fixed_mul_fixed_2(u_fixed, qqs, PR);
//                    v_fixed = fixed_mul_fixed_2(v_fixed, qqs, PR);
                }


                // we round the math, which greatly improves things
                int u_i = fixed_to_int_2(u_fixed, PR);
                int v_i = fixed_to_int_2(v_fixed, PR);
//                int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
//                int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                int index_bmp = (bmp_width*v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity_t);

//                drawPixel(0xFF, index+p.x);

            } else if(antialias) {;// if(false){
                // any of the distances are negative, we are outside.
                // test if we can anti-alias
                // take minimum of all meta distances

                int distance = std::min({w0, w1, w2});
                int delta = (distance) + max_distance_scaled_space_anti_alias;

                if (delta >= 0) {
                    // we need to clip uv coords if they overflow dimension of texture so we
                    // can get the last texel of the boundary
                    // I don't round since I don't care about it here

                    fixed_signed u_fixed = w0_u + w1_u + w2_u + half;
                    fixed_signed v_fixed = w0_v + w1_v + w2_v + half;

                    if(perspective_correct) {
                        // this optimizes fixed_one_over_fixed_2(w0_q + w1_q + w2_q, PR),
                        // I basically save two shifts
                        fixed_signed q_fixed = (w0_q + w1_q + w2_q + 0)>>(-PR+PR_Q);
                        fixed_signed qqs = one_extended/q_fixed;

//                        fixed_signed qqs = one_extended/(w0_q + w1_q + w2_q);

                        u_fixed = fixed_mul_fixed_2(u_fixed, qqs, PR);
                        v_fixed = fixed_mul_fixed_2(v_fixed, qqs, PR);
                    }

                    int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
                    int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                    int index_bmp = bmp_width *v_i + u_i;

                    uint8_t blend = ((long)((delta) << (8 - bits_distance)))>>PR;

                    if (opacity_t < _max_alpha_value) {
                        blend = (blend * opacity_t) >> 8;
                    }

                    color_t col_bmp;
                    bmp.decode(index_bmp, col_bmp);
                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }

            // One step to the right
            w0_u += A01_u2;
            w1_u += A12_u0;
            w2_u += A20_u1;

            w0_v += A01_v2;
            w1_v += A12_v0;
            w2_v += A20_v1;

            if(perspective_correct) {
                w0_q += A01_q2;
                w1_q += A12_q0;
                w2_q += A20_q1;
            }

            w0 += A01;
            w1 += A12;
            w2 += A20;
        }

        // One row step
        w0_row_u += B01_u2;
        w1_row_u += B12_u0;
        w2_row_u += B20_u1;

        w0_row_v += B01_v2;
        w1_row_v += B12_v0;
        w2_row_v += B20_v1;

        if(perspective_correct) {
            w0_row_q += B01_q2;
            w1_row_q += B12_q0;
            w2_row_q += B20_q1;
        }

        w0_row += B01;
        w1_row += B12;
        w2_row += B20;

        index += _width;
    }

}
*/

```

### block tmapper protoype
```
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias,
        typename Sampler,
        typename P2, typename CODER2>
void
canvas<P, CODER>::drawTriangleFast(const bitmap<P2, CODER2> & bmp,
                               const float v0_x, const float v0_y, float u0, float v0,
                               const float v1_x, const float v1_y, float u1, float v1,
                               const float v2_x, const float v2_y, float u2, float v2,
                               const uint8_t opacity_t,
                               bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {

    uint8_t prec_pixel = 4;
    uint8_t prec_uv = 5;
    fixed_signed v0_x_ = float_to_fixed_2(v0_x, prec_pixel);
    fixed_signed v0_y_ = float_to_fixed_2(v0_y, prec_pixel);
    fixed_signed v1_x_ = float_to_fixed_2(v1_x, prec_pixel);
    fixed_signed v1_y_ = float_to_fixed_2(v1_y, prec_pixel);
    fixed_signed v2_x_ = float_to_fixed_2(v2_x, prec_pixel);
    fixed_signed v2_y_ = float_to_fixed_2(v2_y, prec_pixel);

    fixed_signed u0_ = float_to_fixed_2(u0, prec_uv);
    fixed_signed v0_ = float_to_fixed_2(v0, prec_uv);
    fixed_signed u1_ = float_to_fixed_2(u1, prec_uv);
    fixed_signed v1_ = float_to_fixed_2(v1, prec_uv);
    fixed_signed u2_ = float_to_fixed_2(u2, prec_uv);
    fixed_signed v2_ = float_to_fixed_2(v2, prec_uv);
    fixed_signed q_ = float_to_fixed_2(1.0f, prec_uv);

    drawTriangleFast<BlendMode, PorterDuff, antialias, false, Sampler>(bmp,
            v0_x_, v0_y_, u0_, v0_, q_,
            v1_x_, v1_y_, u1_, v1_, q_,
            v2_x_, v2_y_, u2_, v2_, q_,
            opacity_t, prec_pixel, prec_uv,
            aa_first_edge, aa_second_edge, aa_third_edge);

}

template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
        bool antialias, bool perspective_correct,
        typename Sampler,
        typename P2, typename CODER2>
void
canvas<P, CODER>::drawTriangleFast(const bitmap<P2, CODER2> & bmp,
                               const fixed_signed v0_x, const fixed_signed v0_y, fixed_signed u0, fixed_signed v0, fixed_signed q0,
                               const fixed_signed v1_x, const fixed_signed v1_y, fixed_signed u1, fixed_signed v1, fixed_signed q1,
                               const fixed_signed v2_x, const fixed_signed v2_y, fixed_signed u2, fixed_signed v2, fixed_signed q2,
                               const uint8_t opacity_t, const uint8_t sub_pixel_precision, const uint8_t uv_precision,
                               bool aa_first_edge, bool aa_second_edge, bool aa_third_edge) {

    fixed_signed area = functions::orient2d({v0_x, v0_y}, {v1_x, v1_y}, {v2_x, v2_y}, sub_pixel_precision);
    int bmp_width = bmp.width();

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

    const int block = 8;
    maxX += block;
    maxY += block;

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

    uint8_t MAX_PREC = 50;
    uint8_t LL = MAX_PREC - (sub_pixel_precision + BITS_UV_COORDS);
    uint64_t ONE = ((uint64_t)1)<<LL;
    uint64_t one_area = (ONE) / area;

    // PR seems very good for the following calculations
    // Triangle setup
    // this needs at least (P+1) bits, since the delta is always <= length
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

    // this can produce a 2P bits number if the points form a a perpendicular triangle
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
        int PP = PREC_DIST;

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
                int w0_row_ = w0;
                int w1_row_ = w1;
                int w2_row_ = w2;

                for(int iy = p.y; iy < p.y + block; iy++) {
                    int w0_ = w0_row_;
                    int w1_ = w1_row_;
                    int w2_ = w2_row_;

                    for(int ix = p.x; ix < p.x + block; ix++) {
//                        blendColor<BlendMode, PorterDuff>(color_int, (stride + ix), opacity_t);

                        int u_i, v_i;
                        uint64_t u_fixed = (((uint64_t)((uint64_t)w0_*u2 + (uint64_t)w1_*u0 + (uint64_t)w2_*u1)));
                        uint64_t v_fixed = (((uint64_t)((uint64_t)w0_*v2 + (uint64_t)w1_*v0 + (uint64_t)w2_*v1)));

                        if(perspective_correct) {
                            uint64_t q_fixed =(((uint64_t)((uint64_t)w0_*q2 + (uint64_t)w1_*q0 + (uint64_t)w2_*q1)));
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

                        blendColor<BlendMode, PorterDuff>(col_bmp, stride + ix, opacity_t);


                        w0_ += A01;
                        w1_ += A12;
                        w2_ += A20;

                    }

                    w0_row_ += B01;
                    w1_row_ += B12;
                    w2_row_ += B20;
                    stride += _width;
                }

            }



            /*
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

                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity_t);

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

                    if (opacity_t < _max_alpha_value)
                        blend = (blend * opacity_t) >> 8;

                    blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, blend);
                }

            }
             */

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

```

```

/////

//https://stackoverflow.com/questions/31117497/fastest-integer-square-root-in-the-least-amount-of-instructions

    static const uint8_t debruijn32[32] = {
            15,  0, 11, 0, 14, 11, 9, 1, 14, 13, 12, 5, 9, 3, 1, 6,
            15, 10, 13, 8, 12,  4, 3, 5, 10,  8,  4, 2, 7, 2, 7, 6
    };

/* based on CLZ emulation for non-zero arguments, from_sampler
 * http://stackoverflow.com/questions/23856596/counting-leading-zeros-in-a-32-bit-unsigned-integer-with-best-algorithm-in-c-pro
 */
    inline uint8_t shift_for_msb_of_sqrt(uint32_t x) {
        x |= x >>  1;
        x |= x >>  2;
        x |= x >>  4;
        x |= x >>  8;
        x |= x >> 16;
        x++;
        return debruijn32 [x * 0x076be629 >> 27];
    }

    inline uint32_t sqrt_int_2(uint32_t n) {
        if (n==0) return 0;

        uint32_t s = shift_for_msb_of_sqrt(n);
        uint32_t c = 1 << s;
        uint32_t g = c;

        switch (s) {
            case 9:
            case 5:
                if (g*g > n) {
                    g ^= c;
                }
                c >>= 1;
                g |= c;
            case 15:
            case 14:
            case 13:
            case 8:
            case 7:
            case 4:
                if (g*g > n) {
                    g ^= c;
                }
                c >>= 1;
                g |= c;
            case 12:
            case 11:
            case 10:
            case 6:
            case 3:
                if (g*g > n) {
                    g ^= c;
                }
                c >>= 1;
                g |= c;
            case 2:
                if (g*g > n) {
                    g ^= c;
                }
                c >>= 1;
                g |= c;
            case 1:
                if (g*g > n) {
                    g ^= c;
                }
                c >>= 1;
                g |= c;
            case 0:
                if (g*g > n) {
                    g ^= c;
                }
        }

        /* now apply one or two rounds of Newton's method */
        switch (s) {
            case 15:
            case 14:
            case 13:
            case 12:
            case 11:
            case 10:
                g = (g + n/g) >> 1;
            case 9:
            case 8:
            case 7:
            case 6:
                g = (g + n/g) >> 1;
        }

        /* correct potential error at m^2-1 for Newton's method */
        return (g==65536 || g*g>n) ? g-1 : g;
    }

////

```

```
        static
        float xProd(const vertex &p, vertex &q, vertex &r) {
            return q.x*(r.y - p.y) + p.x*(q.y - r.y) + r.x*(p.y - q.y);
        }
```


```c++
// rounded rectangle
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void canvas<P, CODER>::drawRoundedRect_internal(const color_f_t &color,
                                       int left, int top,
                                       int right, int bottom, int radius,
                                       precision sub_pixel_precision,  canvas::opacity_t opacity) {

    color_t col_bmp{};
    const precision p = sub_pixel_precision;
    const int stroke = (1<<p)/1;
    const int aa_range = (1<<p)/1;
    const int radius_squared=(l64(radius)*(radius))>>p;
    const int stroke_radius = (l64(radius-stroke)*(radius-stroke))>>p;
    const int outer_aa_radius = (l64(radius+aa_range)*(radius+aa_range))>>p;
    const int outer_aa_bend = outer_aa_radius-radius_squared;
    const int inner_aa_radius = (l64(radius-stroke-aa_range)*(radius-stroke-aa_range))>>p;
    const int inner_aa_bend = stroke_radius-inner_aa_radius;
    const bool apply_opacity = opacity!=255;

    int max = (1u<<p) - 1;
    int left_   = left;//functions::max(left, (int)0);
    int top_    = top;//functions::max(top, ( int)0);
    int right_  = right;//functions::min(right, (width()-1)<<sub_pixel_precision);
    int bottom_ = bottom;//functions::min(bottom, (height()-1)<<sub_pixel_precision);
//    bool degenerate= left_==right_ || top_==bottom_;
//    if(degenerate) return;
    // intersections
//    u0 = u0+((u1-u0) *(left_-left))/(right-left);
//    v0 = v0+((v1-v0) *(top_-top))/(bottom-top);
//    u1 = u0+((u1-u0) *(right_-left))/(right-left);
//    v1 = v0+((v1-v0) *(bottom_-top))/(bottom-top);
    // round and convert to raster space
//    left_   = (max + left_  )>>p;
//    top_    = (max + top_   )>>p;
//    right_  = (max + right_ )>>p;
//    bottom_ = (max + bottom_)>>p;
    const int step = int(1)<<p;
    const int half = step>>1;
//    degenerate= left_==right_ || top_==bottom_;
//    if(degenerate) return;
    // MULTIPLYING with texture dimensions and doubling precision, helps with the z-fighting
//    int du = ((u1-u0)<<uv_precision) / (right_ - left_);
//    int dv = -((v1-v0)<<uv_precision) / (bottom_ - top_);
//    int u_start = u0<<uv_precision;
//    int u = u_start;
//    int v = -dv*(bottom_ - top_);
    int index = top_ * _width;
//    const precision pp = uv_precision<<1;
    color_t black={255,0,0, 128};
    color_t red={255,0,0};
    const int w_half = (right_-left_)>>1;
    const int h_half = (bottom_-top_)>>1;
    // fix them for accuracy
    const int right_final= left_ + (w_half<<1);
    const int bottom_final= top_ + (h_half<<1);

    const int top_radius = -0+top_, bottom_radius= top_ + (radius);
    const int left_radius = -0+left_, right_radius= left_radius + (radius);
//    if((x)>=0 && (x)<_width && (y)>=0 && (y)<_height)
//    if((x)>=0 && (x)<_width && (y)>=0 && (y)<_height) \
//
#define g1(x, y, c, o) \
            if((x)>=0 && (x)<_width && (y)>=0 && (y)<_height) \
                blendColor<BlendMode, PorterDuff>((c), (x), (y), (o)); \

#define g2(x, y, c, o) { \
            g1((x)>>p,                     (y)>>p,                     (c), (o)) \
            g1((right_final-(x-left_))>>p, (y)>>p,                     (c), (o)) \
            g1((x)>>p,                     (bottom_final-(y-top_))>>p, (c), (o)) \
            g1((right_final-(x-left_))>>p, (bottom_final-(y-top_))>>p, (c), (o)) \
            } \

//     draw top-left

    for (int y = top_; y <= top_+h_half; y+=step) {
        for (int x = left_; x <= left_+w_half; x+=step) {

            const bool inside_rounded_part = x<=(left_+radius) && y<=(top_+radius);

            if(inside_rounded_part) {
                int dx = x-0 - right_radius, dy = y-0 - bottom_radius;
                const int distance_squared = ((l64(dx) * dx) >> p) + ((l64(dy) * dy) >> p);
                const bool inside_radius = (distance_squared - radius_squared) <= 0;
                if (inside_radius) {
                    g2(x, y, red, opacity)
                    const bool inside_stroke_disk = (distance_squared - stroke_radius) >= 0;
                    if (inside_stroke_disk) // inside stroke disk
                    g2(x, y, black, opacity)
                    else { // outside stroke disk, let's test_texture for aa disk or radius inclusion
                        const int delta_inner_aa = -inner_aa_radius + distance_squared;
                        const bool inside_inner_aa_ring = delta_inner_aa >= 0;
                        if (antialias && inside_inner_aa_ring) {
                            // scale inner to 8 bit and then convert to integer
                            uint8_t blend = ((delta_inner_aa) << (8)) / inner_aa_bend;
                            if (apply_opacity) blend = (blend * opacity) >> 8;
                            g2(x, y, black, blend);
                        }
                    }
                } else if (antialias) { // we are outside the main radius
                    const int delta_outer_aa = outer_aa_radius - distance_squared;
                    const bool inside_outer_aa_ring = delta_outer_aa >= 0;
                    if (inside_outer_aa_ring) {
                        // scale inner to 8 bit and then convert to integer
                        uint8_t blend = ((delta_outer_aa) << (8)) / outer_aa_bend;
                        if (apply_opacity) blend = (blend * opacity) >> 8;
                        g2(x, y, black, blend);
                    }
                }
            } else {
                g2(x, y, red, opacity);
                if(x>left_+radius) {
                    if(y>=top_ && y<top_+stroke+step) {
                        g2(x, y, black, opacity);
                    }
                }

                if(x<left_radius+stroke+step) {
                    g2(x, y, black, opacity);
                }

            }

        }

    }



#undef g1
#undef g2
}

```

```c++
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff, bool antialias>
void canvas<P, CODER>::drawCircleOLD(const color_f_t & color,
                                  const int centerX,
                                  const int centerY,
                                  const int radius,
                                  const precision sub_pixel_precision,
                                  const opacity_t opacity) {
    color_t color_int;
    const precision p = sub_pixel_precision;
    coder().convert(color, color_int);
    int bits_for_antialias_distance, max_blend_distance=0;
    const int radius_squared=(l64(radius)*radius)>>p;
    int c=0;
    if(antialias) {
        bits_for_antialias_distance = 1;
        max_blend_distance = (1u << bits_for_antialias_distance)<<(p);
        const int b = (l64(radius+max_blend_distance)*(radius+max_blend_distance))>>p;
        c = b - radius_squared;
    }
    bool apply_opacity = opacity!=255;
    int delta;
    // bounding box
    int x_min = centerX - radius - max_blend_distance, y_min = centerY - radius - max_blend_distance;
    int x_max = centerX + radius + max_blend_distance, y_max = centerY + radius + max_blend_distance;
    // clipping
    x_min = functions::max(0, x_min);
    y_min = functions::max(0, y_min);
    x_max = functions::min(width()<<p, x_max);
    y_max = functions::min(height()<<p, y_max);
    const int step = 1<<p;
    // Round start position up to next integer multiple
    // (we sample at integer pixel positions, so if our
    // min is not an integer coordinate, that pixel won't be hit)
    int sub_mask = step-1;
    x_min = (x_min + sub_mask) & (~sub_mask);
    y_min = (y_min + sub_mask) & (~sub_mask);
    x_max = (x_max + sub_mask) & (~sub_mask);
    y_max = (y_max + sub_mask) & (~sub_mask);
    for (int y = y_min; y < y_max; y+=step) {
        for (int x = x_min; x < x_max; x+=step) {
            // 16 bit precision fixed point
            int dx = x-centerX, dy = y-centerY;
            int distance= ((l64(dx)*dx)>>p) + ((l64(dy)*dy)>>p) - radius_squared;
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
    opacity=255;
    const precision p = sub_pixel_precision;
    const l64 stroke = stroke_size;//(10<<p)/1;
    const l64 aa_range = (1<<p)/1;
    const l64 radius_squared=(l64(radius)*(radius))>>p;
    const l64 stroke_radius = (l64(radius-stroke)*(radius-stroke))>>p;
    const l64 outer_aa_radius = (l64(radius+aa_range)*(radius+aa_range))>>p;
    const l64 outer_aa_bend = outer_aa_radius-radius_squared;
    const l64 inner_aa_radius = (l64(radius-stroke-aa_range)*(radius-stroke-aa_range))>>p;
    const l64 inner_aa_bend = stroke_radius-inner_aa_radius;
    const bool apply_opacity = opacity!=255;
    const l64 mask= (1<<sub_pixel_precision)-1;
    // dimensions in two spaces, one in raster spaces for optimization
    const l64 step = (l64(1)<<p);
    const l64 half = 0;//step>>1;
    const l64 left_=(left+step), top_=(top+step), right_=(right-step), bottom_=(bottom-step);
//    const l64 left_=(left+0), top_=(top+0), right_=(right-0), bottom_=(bottom-0);
    const l64 left_r=left_>>p, top_r=top_>>p, right_r=right_>>p, bottom_r=bottom_>>p;
    bool degenerate= left_r==right_r || top_r==bottom_r;
    if(degenerate) return;
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
    l64 x_1=(x), y_1=(y), x_2=(right_r-(x_1-left_r)), y_2= (bottom_r-(y_1-top_r)); \
            g1(x_1, y_1, u,       v,       (s), (o)) \
            g1(x_2, y_1, u0+(right_r-x_1)*du, v,       (s), (o)) \
            g1(x_1, y_2, u,       v0+(bottom_r-y_1)*dv, (s), (o)) \
            g1(x_2, y_2, u0+(right_r-x_1)*du,       v0+(bottom_r-y_1)*dv, (s), (o)) \
            } \

    // this draws the four rounded parts, it could be faster but then it will also be
    // much complex.
    const l64 ax= (left_+radius);
    const l64 ay= (top_+radius);
//    for (l64 y = (top_)&~0, v=v0, yh=top_r; yh < ((ay-0)>>p); y+=step, v+=dv, yh++) {
//        for (l64 x = (left_)&~0, u=u0, xh=left_r; xh < ((ax-0)>>p); x+=step, u+=du, xh++) {
    for (l64 y = (top_)&~mask, v=v0, yh=top_r; yh < ((ay-0)>>p); y+=step, v+=dv, yh++) {
        for (l64 x = (left_)&~mask, u=u0, xh=left_r; xh < ((ax-0)>>p); x+=step, u+=du, xh++) {
            l64 dx = x+0- half -  (ax), dy = y+0- half -(ay);
            const l64 distance_squared = ((l64(dx) * dx) >> p) + ((l64(dy) * dy) >> p);
            const bool inside_radius = (distance_squared - radius_squared) <= 0;
            if (inside_radius) {
                g2(xh, yh, u, v, sampler_fill, opacity)
                const bool inside_stroke_disk = (distance_squared - stroke_radius) >= 0;
                if (inside_stroke_disk) // inside stroke disk
                    g2(xh, yh, u, v, sampler_stroke, opacity)
                else { // outside stroke disk, let's test for aa disk or radius inclusion
                    const int delta_inner_aa = -inner_aa_radius + distance_squared;
                    const bool inside_inner_aa_ring = delta_inner_aa >= 0;
                    if (antialias && inside_inner_aa_ring) {
                        // scale inner to 8 bit and then convert to integer
                        uint8_t blend = ((delta_inner_aa) << (8)) / inner_aa_bend;
                        if (apply_opacity) blend = (blend * opacity) >> 8;
                        g2(xh, yh, u, v, sampler_stroke, blend);
                    }
                }
            } else if (antialias) { // we are outside the main radius
                const int delta_outer_aa = outer_aa_radius - distance_squared;
                const bool inside_outer_aa_ring = delta_outer_aa >= 0;
                if (inside_outer_aa_ring) {
                    // scale inner to 8 bit and then convert to integer
                    uint8_t blend = ((delta_outer_aa) << (8)) / outer_aa_bend;
                    if (apply_opacity) blend = (blend * opacity) >> 8;
                    g2(xh, yh, u, v, sampler_stroke, blend);
                }
            }
        }
    }
//return;
#undef g1
#undef g2
#define maX(a, b) functions::max<l64>(a, b)
#define miN(a, b) functions::min<l64>(a, b)
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
        const l64 ll=(left_+radius), tt=top_, rr=right_-radius+step, bb=top_+radius;
        const l64 ll_r= maX(0, ll>>p), tt_r= maX(0, (tt)>>p), rr_r= miN(_width-1, rr>>p), bb_r= miN(_height-1, bb>>p);
        int index = tt_r * _width; const l64 u_start=u0 + du*(ll_r-left_r); l64 v= v0 + dv*(tt_r-top_r);
        for (l64 y=tt_r, yy=tt; y<bb_r; y++, yy+=step, v+=dv, index+=_width) {
            for (l64 x=ll_r, u=u_start; x<=rr_r; x++, u+=du) {
                sampler_fill.sample(u, v, uv_p, color);
                blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                if(yy<=tt+stroke) {
                    sampler_stroke.sample(u, v, uv_p, color);
                    blendColor<BlendMode, PorterDuff>(color, (index+x), opacity);
                }
            }
        }
    }
    { // bottom
        const int ll=left_+radius, tt=bottom_-radius+0, rr=right_-radius+step, bb=bottom_;
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
        const int ll=left_, tt=top_+radius, rr=ll+radius, bb=bottom_-radius+step;
        const int ll_r= maX(0, ll>>p), tt_r= maX(0, tt>>p), rr_r= miN(_width-1, rr>>p), bb_r= miN(_height-1, bb>>p);
        int index = tt_r * _width; const l64 u_start=u0 + du*(ll_r-left_r); l64 v= v0 + dv*(tt_r-top_r);
        for (int y=tt_r, yy=tt&~mask; y<=bb_r; y++, yy+=step, v+=dv, index+=_width) {
            for (int x=ll_r, xx=ll&~mask, u=u_start; x<rr_r; x++, xx+=step, u+=du) {
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
        const int ll=right_-radius, tt=top_+radius, rr=right_, bb=bottom_-radius+step;
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

```


```c++

template<typename P, typename CODER>
void canvas<P, CODER>::fxaa2(int left, int top, int right, int bottom) {
//    left=160;top=160;right=left+300;bottom=top+300;
//return;
    // this is the config in an optimized manner
    const l64 FXAA_SPAN_PIXELS_MAX = 8; // max pixels span
    const l64 FXAA_REDUCE_MUL_BITS = 3; // to be used as 1/2^3
    const l64 FXAA_REDUCE_MIN_BITS = 7; // to be used as 1/2^7
    const l64 LUMA_THRESHOLD_BITS = 5; // to be used as 1/2^5

    const char p = 12;
    const char t = p - coder().g();
    const l64 ONE = l64(1) << p;
    const l64 FXAA_REDUCE_MIN = ONE >> FXAA_REDUCE_MIN_BITS;
    const l64 FXAA_SPAN_MAX = FXAA_SPAN_PIXELS_MAX << p;

    const l64 fxaaConsoleEdgeThresholdBits = 3;
    const l64 fxaaConsoleEdgeThresholdMin = 0;
    const l64 fxaaConsoleRcpFrameOpt = 1;
    const l64 fxaaConsoleRcpFrameOpt2 = 1;
    const l64 fxaaConsoleEdgeSharpness = 2;
    const l64 epsilon = ONE/384;
    const int pitch = width();
    for (int yy = top, index = top * pitch; yy < bottom; ++yy, index += pitch) {
        for (int xx = left; xx < right; ++xx) {
            color_t nw, ne, sw, se, m, rgb_N1, rgb_P1, rgb_N2, rgb_P2, rgb_3, rgb_4, rgb_A, rgb_B;
            this->_bitmap_canvas->decode(index + xx, m);
            this->_bitmap_canvas->decode(index + xx - pitch - 1, nw);
            this->_bitmap_canvas->decode(index + xx - pitch + 1, ne);
            this->_bitmap_canvas->decode(index + xx + pitch - 1, sw);
            this->_bitmap_canvas->decode(index + xx + pitch + 1, se);
            // convert lumas to p bits space
            l64 luma_M = l64(m.g) << t, luma_NW = l64(nw.g) << t, luma_NE = epsilon+(l64(ne.g) << t),
                    luma_SW =l64(sw.g) << t, luma_SE = l64(se.g) << t;
            l64 luma_min = microgl::functions::min(luma_NW, luma_NE, luma_SW, luma_SE);
            l64 luma_max = microgl::functions::max(luma_NW, luma_NE, luma_SW, luma_SE);
            l64 lumaMaxScaled = luma_max >> fxaaConsoleEdgeThresholdBits;
            l64 lumaMinM = microgl::functions::min(luma_min, luma_M);
            l64 lumaMaxScaledClamped = microgl::functions::max(fxaaConsoleEdgeThresholdMin, lumaMaxScaled);
            l64 lumaMaxM = microgl::functions::max(luma_max, luma_M);
            l64 lumaMaxSubMinM = lumaMaxM - lumaMinM;
            // If contrast is lower than a maximum threshold ...
            if (lumaMaxSubMinM < (lumaMaxScaledClamped))
                continue;

            l64 dirSwMinusNe = luma_SW - luma_NE;
            l64 dirSeMinusNw = luma_SE - luma_NW;
            l64 dir_x = dirSwMinusNe + dirSeMinusNw;
            l64 dir_y = dirSwMinusNe - dirSeMinusNw;
            if (dir_x == 0 && dir_y == 0) continue;
            // dir 1
            {
                const l64 length= microgl::math::length(dir_x, dir_y);
                l64 dir1_x= ((dir_x << p) / length) >> fxaaConsoleRcpFrameOpt;
                l64 dir1_y= ((dir_y << p) / length) >> fxaaConsoleRcpFrameOpt;
                this->_bitmap_canvas->decode(xx - (dir1_x>>p), yy - (dir1_y>>p), rgb_N1);
                this->_bitmap_canvas->decode(xx + (dir1_x>>p), yy + (dir1_y>>p), rgb_P1);
                l64 dirAbsMinTimesC = microgl::functions::min(microgl::math::abs(dir1_x),
                        microgl::math::abs(dir1_y)) * fxaaConsoleEdgeSharpness;
                l64 dir2_x = 2*microgl::functions::clamp((dir1_x << p) / dirAbsMinTimesC, -(2ll << p), 2ll << p);
                l64 dir2_y = 2*microgl::functions::clamp((dir1_y << p) / dirAbsMinTimesC, -(2ll << p), 2ll << p);
                this->_bitmap_canvas->decode(xx - (dir2_x>>p), yy - (dir2_y>>p), rgb_N2);
                this->_bitmap_canvas->decode(xx + (dir2_x>>p), yy + (dir2_y>>p), rgb_P2);

            }
            rgb_A.r = (int(rgb_N1.r) + int(rgb_P1.r)) >> 1, rgb_A.g = (int(rgb_N1.g) + int(rgb_P1.g)) >> 1;
            rgb_A.b =(int(rgb_N1.b) + int(rgb_P1.b)) >> 1;
            rgb_B.r = int(rgb_A.r >> 1) + ((int(rgb_N2.r) + int(rgb_P2.r)) >> 2); // compute the average of 4 pixels
            rgb_B.g = int(rgb_A.g >> 1) + ((int(rgb_N2.g) + int(rgb_P2.g)) >> 2);
            rgb_B.b = int(rgb_A.b >> 1) + ((int(rgb_N2.b) + int(rgb_P2.b)) >> 2);
            l64 luma_rgb_B = l64(rgb_B.g) << t;
            color_t *selected_color;
            if (luma_rgb_B < luma_min || luma_rgb_B > luma_max)
                selected_color = &rgb_A;
            else
                selected_color = &rgb_B;
//            selected_color=&rgb_B;
            selected_color->a = m.a; // restore middle alpha
            P output;
            color_t black{0, 0, 0};
//            selected_color=&black;
            coder().encode(*selected_color, output);
            drawPixel(output, index + xx);
        }
    }
}
```
```c++
    //    static constexpr uint8_t size=123;//uint32_t(1)<<bits1;
//    constexpr static Table tab=MagicFunction<func>(gen_seq<10>{});

//    struct Table {
//         uint8_t data[300];
//    };


//    template<typename T>
//    static constexpr T look_up_table_elem (int i) {
//        return {};
//    }
//
//    template<>
//    static constexpr int look_up_table_elem (int i) {
//        return i;
//    }
//
//    template<typename T, int... N>
//    struct lookup_table_expand{};
//
//    template<typename T, int... N>
//    struct lookup_table_expand<T, 1, N...> {
//        static constexpr Table values = {{ look_up_table_elem<T>(0), N... }};
//    };
//
//    template<typename T, int L, int... N>
//    struct lookup_table_expand<T, L, N...>: lookup_table_expand<T, L-1, look_up_table_elem<T>(L-1), N...> {};
//
//    static constexpr Table tab = lookup_table_expand<int , 150>::values ;


//    template<int... Is> struct seq{
//        static const unsigned size= sizeof...(Is);
//    };
//
//    template<int N, int... Is>
//    struct gen_seq : gen_seq<(N)-(1), (N)-(1), (Is)...>{};
//
//    template<int... Is>
//    struct gen_seq<(0), Is...> : seq<(Is)...>{};
//
//
//    static constexpr uint8_t f(int i){ return i; /*lazy*/ }
//
//    template<int... Is>
//    constexpr static Table MagicFunction(seq<Is...>){
//        return {{ f(Is)... }};
//    }
//
////    static constexpr Table tab=MagicFunction(gen_seq<uint8_t(size-1)>{});
//    constexpr static int tab2[128]{gen_seq<10>{}};
//
//    constexpr static Table tab=MagicFunction(gen_seq<10>{});
//public:
//    static uint8_t get(const uint8_t & val) {
//        return tab._[val];
//    };
```