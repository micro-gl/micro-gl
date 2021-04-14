#pragma once

#include <microgl/rgba_t.h>
#include <microgl/vec2.h>

namespace microgl {
    namespace sampling {

//        float sdSegment( in vec2 p, in vec2 a, in vec2 b )
//        {
//            vec2 pa = p-a, ba = b-a;
//            float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
//            return length( pa - ba*h );
//        }

        template <typename number, typename rgba_=rgba_t<8,8,8,8>,
                  bool useBigIntegers=true>
        struct d1_function_sampler {
            using rgba = rgba_;
            using rint_big=int64_t;
            static constexpr precision_t p_bits= 24;
            using rint= typename microgl::traits::conditional<p_bits>=16,
                                    int64_t, int32_t>::type;
            using vertex = vec2<number>;
            using ivertex = vec2<rint>;
            static constexpr precision_t p_bits_double= p_bits<<1;
            static constexpr rint ONE= rint(1)<<p_bits;
            static constexpr rint HALF= ONE>>1;

            struct segment_t {
                ivertex p;
                rint reciprocal_dis_ab=0; // 1/dot(p1, p2)
            };
            segment_t * _seg=nullptr;

        public:
            d1_function_sampler() = default;
            ~d1_function_sampler() {
                delete [] _seg;
            }

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

            unsigned _size=0;
            color_t color1= {255,0,0, 255};
            rint to_index=0;
            rint epsilon;
            void updatePoints(vertex * vertices, unsigned size, number $epsilon) {
                delete [] _seg;
                _size=size;
                _seg = new segment_t[size];
                epsilon = microgl::math::to_fixed(($epsilon/number(2))*($epsilon/number(2)), p_bits);
                int ix=0;
                to_index=0;
                while (ix < size) {
                    _seg[to_index].p.x = microgl::math::to_fixed(vertices[ix].x, p_bits);
                    _seg[to_index].p.y = microgl::math::to_fixed(vertices[ix].y, p_bits);
                    if(to_index==0){
                        to_index++;
                    } else if(to_index>=1) {
                        auto vec = _seg[to_index].p-_seg[to_index-1].p;
                        auto th = ((vec.x*vec.x)>>p_bits) + ((vec.y*vec.y)>>p_bits);
                        if(th>1) {
                            auto vec2 = vertices[ix]-vertices[ix-1];
                            auto dot = microgl::math::sqrt(vec2.dot(vec2));
                            auto recip = number(1)/dot;
                            _seg[to_index].reciprocal_dis_ab = microgl::math::to_fixed(recip, p_bits);
//                            const rint dot = ((vec.x*vec.x)>>p_bits) +
//                                             ((vec.y*vec.y)>>p_bits);
//                            _seg[to_index].reciprocal_dis_ab = (ONE/1) /
//                                    microgl::math::sqrt(dot<<(0));
                            to_index++;
                        }
                    }
                    ix++;
                }
            }

            int clamp(const int &v, const int &e0, const int &e1) const
            {
                return v<e0 ? e0 : (v<e1 ? v : e1);
            }

            mutable rint latest_u=-1;
            mutable rint latest_index=0;
#define aaaa(x) ((x)<0?-(x):(x))

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                rint dis=ONE;
                ivertex p{u_tag, v_tag};
                int ix = u_tag>=latest_u ? latest_index : 0;
                for (; ix < to_index; ++ix) {
                    if(p.x<_seg[ix+1].p.x) break;
                }
                latest_u=u_tag;
                latest_index=ix;
//                ix+=1;

                int start = ix-1<=0 ? 0 : ix-1;
//                for (int jx = ix; jx < ix+1; ++jx) {
                for (int jx = 0; jx < 7; ++jx) {
                    const ivertex & a = _seg[jx].p;
                    const ivertex & b = _seg[jx+1].p;
                    ivertex ab = b-a;
//                ivertex ap = p - a;
                    rint signed_area= -(((p.x-a.x)*ab.y)>>p_bits) + (((p.y-a.y)*ab.x)>>p_bits);
                    signed_area = aaaa(signed_area);
                    rint dis2 = ((signed_area)*_seg[jx+1].reciprocal_dis_ab)>>(p_bits);
                    if(dis2<dis) dis=dis2;
                }
//                const ivertex & a = _seg[ix].p;
//                const ivertex & b = _seg[ix+1].p;
//                ivertex ab = b-a;
////                ivertex ap = p - a;
//                rint signed_area= -(((p.x-a.x)*ab.y)>>p_bits) + (((p.y-a.y)*ab.x)>>p_bits);
//                const rint h = (aaaa(signed_area)*_seg[ix+1].reciprocal_dis_ab)>>(p_bits);
//                bool right_of = h<=0;

                //

                constexpr rint aa_bits = p_bits - 10 < 0 ? 0 : p_bits - 10;
                constexpr rint aa_band = 1u << aa_bits;
                constexpr rint stroke_band = ONE/20;

                dis = aaaa(dis)- stroke_band;

                output={255,0,0,0};

                if((dis)<=0) {
                    output={0,0,0, 255};
                }
//                else if (dis < aa_band) {
//                    const unsigned char factor = ((color1.a*(aa_band-dis)) >> aa_bits);
//                    output={0,0,0, factor};
//                }

                //
//                output={255,0,0,255};
//                if(right_of)
//                    output={255,255,255,255};

//                if((dis)<=0) {
//                    output={0,0,0, 255};
//                } else if (dis < aa_band) {
//                    const unsigned char factor = ((color1.a*(aa_band-dis)) >> aa_bits);
//                    output={0,0,0, factor};
//                }

            }
#undef aaaa

        private:
        };

    }
}