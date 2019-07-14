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