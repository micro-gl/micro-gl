#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>
#include <microgl/Bitmap.h>

namespace microgl {
    namespace sampling {
        enum class texture_sampling {
            NearestNeighboor, Bilinear
        };

        template <typename P, typename CODER, texture_sampling default_sampling=texture_sampling::NearestNeighboor>
        class texture : public sampler<texture<P, CODER, default_sampling>> {
            using base= sampler<texture<P, CODER, default_sampling>>;
            using l64= long long;
        public:
            texture() : base{CODER::red_bits(),CODER::green_bits(),CODER::blue_bits(),CODER::alpha_bits()} {};
            explicit texture(Bitmap<P, CODER> * bitmap) :
                    _bmp{bitmap}, texture() {};

            void updateBitmap(Bitmap<P, CODER> * bitmap) {
                _bmp=bitmap;
            }

            inline void sample(const l64 u, const l64 v,
                               const uint8_t bits,
                               color_t &output) const {
                // compile time branching for default sampling
                if(default_sampling==texture_sampling::NearestNeighboor)
                    sample_nearest_neighboor(u, v, bits, output);
                else if(default_sampling==texture_sampling::Bilinear)
                    sample_bilinear(u, v, bits, output);
            }

            inline void sample_nearest_neighboor(const l64 u, const l64 v,
                                        const uint8_t bits, color_t &output) const {
                const l64 half= l64(1)<<(bits-1);
                const l64 x = (l64(_bmp->width()-1)*(u)+half) >> bits;
                const l64 y = (l64(_bmp->height()-1)*(v)+half) >> bits;
                const int index_bmp = y*_bmp->width() + x;
                _bmp->decode(index_bmp, output);
            }

            inline void sample_bilinear(l64 u, l64 v,
                               const uint8_t bits, color_t &output) const {
                const l64 bmp_w_max = _bmp->width()-1;
                const l64 bmp_h_max = _bmp->height()-1;
                u=u*bmp_w_max;
                v=v*bmp_h_max;
                const l64 max = l64(1) << bits;
                const l64 max_value = max - 1;
//                l64 mask = ~max_value;
//                l64 integral_sampleU = u & mask;
//                l64 integral_sampleV = v & mask;
//                l64 tx = -round_sampleU + u;
//                l64 ty = -round_sampleV + v;
                // take reminder part
                const l64 tx = u & max_value; // reminder u
                const l64 ty = v & max_value; // reminder v
                const l64 U = (u) >> bits; // integral u
                const l64 V = (v) >> bits; // integral v
                const l64 U_plus_one = U >= bmp_w_max ? U : U + 1;
                const l64 V_plus_one = V >= bmp_h_max ? V : V + 1;

                color_t c00, c10, c01, c11;
                // todo: can optimize indices not to get multiplied
                _bmp->decode(U, V, c00);
                _bmp->decode(U_plus_one, V, c10);
                _bmp->decode(U, V_plus_one, c01);
                _bmp->decode(U_plus_one, V_plus_one, c11);

                color_t a, b, c;

                a.r = (l64(c00.r) * (max - tx) + l64(c10.r) * tx) >> bits;
                a.g = (l64(c00.g) * (max - tx) + l64(c10.g) * tx) >> bits;
                a.b = (l64(c00.b) * (max - tx) + l64(c10.b) * tx) >> bits;
                a.a = (l64(c00.a) * (max - tx) + l64(c10.a) * tx) >> bits;

                b.r = (l64(c01.r) * (max - tx) + l64(c11.r) * tx) >> bits;
                b.g = (l64(c01.g) * (max - tx) + l64(c11.g) * tx) >> bits;
                b.b = (l64(c01.b) * (max - tx) + l64(c11.b) * tx) >> bits;
                b.a = (l64(c01.a) * (max - tx) + l64(c11.a) * tx) >> bits;

                output.r = (l64(a.r) * (max - ty) + l64(b.r) * ty) >> bits;
                output.g = (l64(a.g) * (max - ty) + l64(b.g) * ty) >> bits;
                output.b = (l64(a.b) * (max - ty) + l64(b.b) * ty) >> bits;
                output.a = (l64(a.a) * (max - ty) + l64(b.a) * ty) >> bits;
            }

        private:

            Bitmap<P, CODER> * _bmp= nullptr;
        };

    }
}