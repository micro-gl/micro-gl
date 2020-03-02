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

            inline void sample(const int u, const int v,
                                      const uint8_t bits, color_t &output) const {
                // compile time branching for default sampling
                if(default_sampling==texture_sampling::NearestNeighboor)
                    sample_nearest_neighboor(u, v, bits, output);
                else if(default_sampling==texture_sampling::Bilinear)
                    sample_bilinear(u, v, bits, output);
            }

            inline void sample_nearest_neighboor(const int u, const int v,
                                        const uint8_t bits, color_t &output) const {
                int x = (l64(_bmp->width()-0)*u) >> bits;
                int y = (l64(_bmp->height()-0)*v) >> bits;
                int index_bmp = y*_bmp->width() + x;
                _bmp->decode(index_bmp, output);
            }

            inline void sample_bilinear(l64 u, l64 v,
                               const uint8_t bits, color_t &output) const {

                const int bmp_w_max = _bmp->width()-1;
                const int bmp_h_max = _bmp->height()-1;
                u=u*bmp_w_max;
                v=v*bmp_h_max;
                l64 max = 1u << bits;
                l64 max_value = max - 1;
                l64 mask = ~max_value;
                l64 round_sampleU = u & mask;
                l64 round_sampleV = v & mask;
//            int tx = -round_sampleU + u;
//            int ty = -round_sampleV + v;
                l64 tx = u & max_value;
                l64 ty = v & max_value;
                l64 U = (round_sampleU) >> bits;
                l64 V = (round_sampleV) >> bits;
                l64 U_plus_one = U >= bmp_w_max ? U : U + 1;
                l64 V_plus_one = V >= bmp_h_max ? V : V + 1;

                color_t c00, c10, c01, c11;
                // todo: can optimize indices not to get multiplied
                _bmp->decode(U, V, c00);
                _bmp->decode(U_plus_one, V, c10);
                _bmp->decode(U, V_plus_one, c01);
                _bmp->decode(U_plus_one, V_plus_one, c11);

                color_t a, b, c;

                a.r = (c00.r * (max - tx) + c10.r * tx) >> bits;
                a.g = (c00.g * (max - tx) + c10.g * tx) >> bits;
                a.b = (c00.b * (max - tx) + c10.b * tx) >> bits;
                a.a = (c00.a * (max - tx) + c10.a * tx) >> bits;

                b.r = (c01.r * (max - tx) + c11.r * tx) >> bits;
                b.g = (c01.g * (max - tx) + c11.g * tx) >> bits;
                b.b = (c01.b * (max - tx) + c11.b * tx) >> bits;
                b.a = (c01.a * (max - tx) + c11.a * tx) >> bits;

                output.r = (a.r * (max - ty) + b.r * ty) >> bits;
                output.g = (a.g * (max - ty) + b.g * ty) >> bits;
                output.b = (a.b * (max - ty) + b.b * ty) >> bits;
                output.a = (a.a * (max - ty) + b.a * ty) >> bits;
            }

        private:

            Bitmap<P, CODER> * _bmp= nullptr;
        };

    }
}