#pragma once

#include <microgl/sampler.h>

namespace microgl {
    namespace sampling {
        enum class texture_filter {
            NearestNeighboor, Bilinear
        };

        enum class texture_wrap {
            None, Clamp, ClampToBorderColor, Repeat, MirroredRepeat
        };

        template <typename Bitmap,
                texture_filter filter=texture_filter::NearestNeighboor,
                texture_wrap wrap_u=texture_wrap::None,
                texture_wrap wrap_v=texture_wrap::None>
        class texture : public sampler<texture<Bitmap, filter, wrap_u, wrap_v>> {
            using base= sampler<texture<Bitmap, filter, wrap_u, wrap_v>>;
            using rint= int;
        public:
            texture() : texture{nullptr} {};
            explicit texture(Bitmap * bitmap) :
                    base{bitmap->coder().red_bits(),
                         bitmap->coder().green_bits(),
                         bitmap->coder().blue_bits(),
                         bitmap->coder().alpha_bits()},
                    _border_color{0,0,0, channel((1<<bitmap->coder().alpha_bits())-1)},
                    _bmp{bitmap} {};

            void updateBitmap(Bitmap * bitmap) {
                _bmp=bitmap;
            }

            Bitmap & bitmap() {
                return *_bmp;
            }

            void updateBorderColor(const color_t & color) {
                _border_color=color;
            }
            using base::sample;
            inline void sample(const rint u, const rint v,
                               const uint8_t bits,
                               color_t &output) const {
                rint u_=u, v_=v;
                switch(wrap_u) {
                    case texture_wrap::Clamp : {
                        rint one= rint(1)<<bits;
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
                        rint one= rint(1)<<bits;
                        v_=v<0?0:(v>one ? one : v); break;
                    }
                    case texture_wrap::ClampToBorderColor : {
                        if(v<0 || v>(rint(1)<<bits)) {
                            output=_border_color; return;
                        }
                    }
                    case texture_wrap::Repeat : {
                        v_=v&((rint(1)<<bits)-1);
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

            inline void sample_nearest_neighboor(const rint u, const rint v,
                                        const uint8_t bits, color_t &output) const {
                const rint half= rint(1)<<(bits-1);
                const rint x = (rint(_bmp->width()-1)*(u)+half) >> bits;
                const rint y = (rint(_bmp->height()-1)*(v)+half) >> bits;
                const int index_bmp = y*_bmp->width() + x;
                _bmp->decode(index_bmp, output);
//                output={0,0,0,255};
            }

            inline void sample_bilinear(rint u, rint v,
                               const uint8_t bits, color_t &output) const {
                const rint bmp_w_max = _bmp->width()-1;
                const rint bmp_h_max = _bmp->height()-1;
                u=u*bmp_w_max;
                v=v*bmp_h_max;
                const rint max = rint(1) << bits;
                const rint max_value = max - 1;
//                rint mask = ~max_value;
//                rint integral_sampleU = u & mask;
//                rint integral_sampleV = v & mask;
//                rint tx = -round_sampleU + u;
//                rint ty = -round_sampleV + v;
                // take reminder part
                const rint tx = u & max_value; // reminder u
                const rint ty = v & max_value; // reminder v
                const rint U = (u) >> bits; // integral u
                const rint V = (v) >> bits; // integral v
                const rint U_plus_one = U >= bmp_w_max ? U : U + 1;
                const rint V_plus_one = V >= bmp_h_max ? V : V + 1;

                color_t c00, c10, c01, c11;
                // todo: can optimize indices not to get multiplied
                _bmp->decode(U, V, c00);
                _bmp->decode(U_plus_one, V, c10);
                _bmp->decode(U, V_plus_one, c01);
                _bmp->decode(U_plus_one, V_plus_one, c11);

                color_t a, b, c;

                a.r = (rint(c00.r) * (max - tx) + rint(c10.r) * tx) >> bits;
                a.g = (rint(c00.g) * (max - tx) + rint(c10.g) * tx) >> bits;
                a.b = (rint(c00.b) * (max - tx) + rint(c10.b) * tx) >> bits;
                a.a = (rint(c00.a) * (max - tx) + rint(c10.a) * tx) >> bits;

                b.r = (rint(c01.r) * (max - tx) + rint(c11.r) * tx) >> bits;
                b.g = (rint(c01.g) * (max - tx) + rint(c11.g) * tx) >> bits;
                b.b = (rint(c01.b) * (max - tx) + rint(c11.b) * tx) >> bits;
                b.a = (rint(c01.a) * (max - tx) + rint(c11.a) * tx) >> bits;

                output.r = (rint(a.r) * (max - ty) + rint(b.r) * ty) >> bits;
                output.g = (rint(a.g) * (max - ty) + rint(b.g) * ty) >> bits;
                output.b = (rint(a.b) * (max - ty) + rint(b.b) * ty) >> bits;
                output.a = (rint(a.a) * (max - ty) + rint(b.a) * ty) >> bits;
            }

        private:
            color_t _border_color {0,0,0,(1<<Bitmap::Coder::alpha_bits())-1};
            Bitmap * _bmp= nullptr;
        };

    }
}