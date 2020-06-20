#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>
#include <microgl/Bitmap.h>

namespace microgl {
    namespace sampling {
        enum class texture_filter {
            NearestNeighboor, Bilinear
        };

        enum class texture_wrap {
            None, Clamp, ClampToBorderColor, Repeat, MirroredRepeat
        };

        template <typename P, typename CODER,
                texture_filter filter=texture_filter::NearestNeighboor,
                texture_wrap wrap_u=texture_wrap::None,
                texture_wrap wrap_v=texture_wrap::None>
        class texture : public sampler<texture<P, CODER, filter, wrap_u, wrap_v>> {
            using base= sampler<texture<P, CODER, filter, wrap_u, wrap_v>>;
            using l64= long long;
        public:
            texture() : texture{nullptr} {};
            explicit texture(Bitmap<P, CODER> * bitmap) :
                    base{CODER::red_bits(),CODER::green_bits(),CODER::blue_bits(),CODER::alpha_bits()}, _bmp{bitmap} {};

            void updateBitmap(Bitmap<P, CODER> * bitmap) {
                _bmp=bitmap;
            }

            void updateBorderColor(const color_t & color) {
                _border_color=color;
            }

            inline l64 apply_wrap(const l64 t) {

            }

            inline void sample(const l64 u, const l64 v,
                               const uint8_t bits,
                               color_t &output) const {
                l64 u_=u, v_=v;
                switch(wrap_u) {
                    case texture_wrap::Clamp : {
                        l64 one= 1<<bits;
                        u_=u_<0?0:(u_>one ? one : u_); break;
                    }
                    case texture_wrap::ClampToBorderColor : {
                        if(u<0 || u>(1<<bits)) {
                            output=_border_color; return;
                        }
                    }
                    case texture_wrap::Repeat : {
                        u_=u&((1<<bits)-1);
                        break;
                    }
                    case texture_wrap::None : break;
                }

                switch(wrap_v) {
                    case texture_wrap::Clamp : {
                        l64 one= 1<<bits;
                        v_=v<0?0:(v>one ? one : v); break;
                    }
                    case texture_wrap::ClampToBorderColor : {
                        if(v<0 || v>(1<<bits)) {
                            output=_border_color; return;
                        }
                    }
                    case texture_wrap::Repeat : {
                        v_=v&((l64(1)<<bits)-1);
                        break;
                    }
                    case texture_wrap::None : break;
                }

                // compile time branching for default sampling
                if(filter==texture_filter::NearestNeighboor)
                    sample_nearest_neighboor(u_, v_, bits, output);
                else if(filter==texture_filter::Bilinear)
                    sample_bilinear(u_, v_, bits, output);
            }

            inline void sample_nearest_neighboor(const l64 u, const l64 v,
                                        const uint8_t bits, color_t &output) const {
                const l64 half= l64(1)<<(bits-1);
                const l64 x = (l64(_bmp->width()-1)*(u)+half) >> bits;
                const l64 y = (l64(_bmp->height()-1)*(v)+half) >> bits;
                const int index_bmp = y*_bmp->width() + x;
                _bmp->decode(index_bmp, output);
//                output={0,0,0,255};
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
            color_t _border_color {0,0,0,(1<<CODER::alpha_bits())-1};
            Bitmap<P, CODER> * _bmp= nullptr;
        };

    }
}