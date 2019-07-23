#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemberFunctionCanBeStaticInspection"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma clang diagnostic ignored "-Wconstant-conversion"
#pragma ide diagnostic ignored "OCUnusedStructInspection"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma once

#include "Types.h"
#include "CRPT.h"
#include "Bitmap.h"
#include "Fixed.h"
#include <math.h>
#include<algorithm>

namespace sampler {
#define MAX_VAL_BITS(a) ((1<<(bits)) - 1)

    enum class type {
        None,
        NearestNeighbor,
        Bilinear
    };

    template<typename IMPL>
    class SamplerBase : public CRPT<IMPL> {
    public:

        template<typename P, typename Coder>
        inline static void sample(const Bitmap<P, Coder> &bmp, 
                                  const fixed_signed u, const fixed_signed v, 
                                  const uint8_t bits, color_t & output) {
            IMPL::sample(bmp, u, v, bits, output);
        }

    protected:
        inline static enum type type() {
            return IMPL::type();
        }


    };


    class NearestNeighbor : public SamplerBase<NearestNeighbor> {
    public:
        inline static enum type type() {
            return type::NearestNeighbor;
        }

        template<typename P, typename Coder>
        inline static void sample(const Bitmap<P, Coder> &bmp,
                                  const fixed_signed u, const fixed_signed v,
                                  const uint8_t bits, color_t & output) {

            int h = 0;//(1<<bits) - 1;
            int index_bmp = (bmp.width()*((v+h)>>bits) + ((u+h)>>bits));

            bmp.decode(index_bmp, output);
        }

    };

    class Bilinear : public SamplerBase<Bilinear> {
    public:
        inline static enum type type() {
            return type::Bilinear;
        }

        template<typename P, typename Coder>
        inline static void sample(const Bitmap<P, Coder> &bmp,
                                  const fixed_signed u, const fixed_signed v,
                                  const uint8_t bits, color_t & output) {

            unsigned int bmp_w_max = bmp.width() - 1;
            unsigned int bmp_h_max = bmp.height() - 1;
            fixed_signed sampleU = u;
            fixed_signed sampleV = v;

//                u_i = (bmp_w_max*u_fixed + mask_)>>(BITS_UV_COORDS<<1);
//                v_i = (bmp_h_max*v_fixed + mask_)>>(BITS_UV_COORDS<<1);

            fixed_signed max = ((1<<bits));
            fixed_signed max_value = max-1;
            fixed_signed mask = ~max_value;
//                fixed_signed round_sampleU = (bmp_w_max*u_fixed + 0)>>(resolution);
//                fixed_signed round_sampleV = (bmp_h_max*v_fixed + 0)>>(resolution);
            fixed_signed round_sampleU = (u + 0) & mask;
            fixed_signed round_sampleV = (v + 0) & mask;
            fixed_signed tx = -round_sampleU + sampleU;
            fixed_signed ty = -round_sampleV + sampleV;
            fixed_signed U = (round_sampleU)>>bits;
            fixed_signed V = (round_sampleV)>>bits;
            fixed_signed U_plus_one = U >= (bmp_w_max) ? U : U + 1;
            fixed_signed V_plus_one = V >= bmp_h_max ? V : V+1;

            color_t c00, c10, c01,c11;
            // todo: can optimize indices not to get multiplied
            bmp.decode(U,          V,          c00);
            bmp.decode(U_plus_one, V,          c10);
            bmp.decode(U,          V_plus_one, c01);
            bmp.decode(U_plus_one, V_plus_one, c11);

            color_t a, b, c;

            a.r = (c00.r * (max - tx) + c10.r * tx)>>bits;
            a.g = (c00.g * (max - tx) + c10.g * tx)>>bits;
            a.b = (c00.b * (max - tx) + c10.b * tx)>>bits;

            b.r = (c01.r * (max - tx) + c11.r * tx)>>bits;
            b.g = (c01.g * (max - tx) + c11.g * tx)>>bits;
            b.b = (c01.b * (max - tx) + c11.b * tx)>>bits;

            output.r = (a.r * (max - ty) + b.r * ty)>>bits;
            output.g = (a.g * (max - ty) + b.g * ty)>>bits;
            output.b = (a.b * (max - ty) + b.b * ty)>>bits;
            output.a = 255;

        }

    };

}

#pragma clang diagnostic pop