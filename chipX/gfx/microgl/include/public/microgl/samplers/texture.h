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
        public:
            texture() : base{8,8,8,8} {};
            explicit texture(Bitmap<P, CODER> * bitmap) :
                    _bmp{bitmap}, base{bitmap->coder().red_bits(),
                                       bitmap->coder().green_bits(),
                                       bitmap->coder().blue_bits(),
                                       bitmap->coder().alpha_bits()} {};

            void updateBitmap(Bitmap<P, CODER> * bitmap) {
                _bmp=bitmap;
                base::_red_bits=bitmap->coder().red_bits();
                base::_green_bits=bitmap->coder().green_bits();
                base::_blue_bits=bitmap->coder().blue_bits();
                base::_alpha_bits=bitmap->coder().alpha_bits();
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
                int x = ((_bmp->width()-0)*u) >> bits;
                int y = ((_bmp->height()-0)*v) >> bits;
                int index_bmp = y*_bmp->width() + x;
                _bmp->decode(index_bmp, output);
            }

            inline void sample_bilinear(int u, int v,
                               const uint8_t bits, color_t &output) const {

                const int bmp_w_max = _bmp->width()-1;
                const int bmp_h_max = _bmp->height()-1;
                u=u*bmp_w_max;
                v=v*bmp_h_max;
                int max = 1u << bits;
                int max_value = max - 1;
                int mask = ~max_value;
                int round_sampleU = u & mask;
                int round_sampleV = v & mask;
//            int tx = -round_sampleU + u;
//            int ty = -round_sampleV + v;
                int tx = u & max_value;
                int ty = v & max_value;
                int U = (round_sampleU) >> bits;
                int V = (round_sampleV) >> bits;
                int U_plus_one = U >= bmp_w_max ? U : U + 1;
                int V_plus_one = V >= bmp_h_max ? V : V + 1;

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