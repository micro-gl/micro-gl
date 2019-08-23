```c++
template<typename P, typename CODER>
template<typename BlendMode, typename PorterDuff,
         bool antialias,
         typename P2, typename CODER2>
void
Canvas<P, CODER>::drawTriangle(Bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               const uint8_t opacity) {

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
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

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

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
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
Canvas<P, CODER>::drawQuadrilateral(const Bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               int v3_x, int v3_y, float u3, float v3,
                               const uint8_t opacity) {

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
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

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

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
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
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               int v0_x, int v0_y, float u0, float v0,
                               int v1_x, int v1_y, float u1, float v1,
                               int v2_x, int v2_y, float u2, float v2,
                               const uint8_t opacity) {
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
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

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

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
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
Canvas<P, CODER>::drawTriangle(const Bitmap<P2, CODER2> & bmp,
                               const fixed_signed v0_x, const fixed_signed v0_y, float u0, float v0, float q0,
                               const fixed_signed v1_x, const fixed_signed v1_y, float u1, float v1, float q1,
                               const fixed_signed v2_x, const fixed_signed v2_y, float u2, float v2, float q2,
                               const uint8_t opacity, const uint8_t sub_pixel_precision) {

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

                // we round the numbers, which greatly improves things
                 int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
                 int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                int index_bmp = (bmp_width*v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

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

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
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
void Canvas<P, CODER>::drawCircle(const color_f_t & color,
                                  int centerX, int centerY,
                                  int radius,
                                  uint8_t opacity) {
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

    bool apply_opacity = opacity!=255;
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
                blendColor<BlendMode, PorterDuff>(color_int, index + x, opacity);
            else if(antialias && (delta=c-distance)>=0){

//                 scale inner to 8 bit and then convert to integer
                uint8_t blend = ((delta)<<(8))/c;

                if(apply_opacity)
                    blend = (blend*opacity)>>8;

                blendColor<BlendMode, PorterDuff>(color_int, index + x, blend);
            }

        }

    }

}

```

Old antialias for circle I found
```
template<typename P, typename CODER>
void Canvas<P, CODER>::drawCircle(const color_f_t & color,
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
void Canvas<P, CODER>::drawLine(const color_f_t &color, int x0, int y0, int x1, int y1) {
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
Canvas<P, CODER>::drawTriangle2(const Bitmap<P2, CODER2> & bmp,
                               const fixed_signed v0_x, const fixed_signed v0_y, fixed_signed u0, fixed_signed v0, fixed_signed q0,
                               const fixed_signed v1_x, const fixed_signed v1_y, fixed_signed u1, fixed_signed v1, fixed_signed q1,
                               const fixed_signed v2_x, const fixed_signed v2_y, fixed_signed u2, fixed_signed v2, fixed_signed q2,
                               const uint8_t opacity, const uint8_t sub_pixel_precision, const uint8_t uv_precision) {

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


                // we round the numbers, which greatly improves things
                int u_i = fixed_to_int_2(u_fixed, PR);
                int v_i = fixed_to_int_2(v_fixed, PR);
//                int u_i = clamp(fixed_to_int_2(u_fixed, PR), 0, bmp_w_max);
//                int v_i = clamp(fixed_to_int_2(v_fixed, PR), 0, bmp_h_max);
                int index_bmp = (bmp_width*v_i + u_i);

                color_t col_bmp;
                bmp.decode(index_bmp, col_bmp);
                blendColor<BlendMode, PorterDuff>(col_bmp, index + p.x, opacity);

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

                    if (opacity < _max_alpha_value) {
                        blend = (blend * opacity) >> 8;
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