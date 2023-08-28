/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include <microgl/color.h>
#include <microgl/math/vertex2.h>

namespace microgl {
    namespace sampling {

        /**
         * this sampler receives  2d points in the unit rectangle space,
         * and samples crudely the function they approximate. This is a very fast sampler, that does
         * not allocate space, BUT it comes with some quirks:
         *
         * 1. the spacing of the points should be uniform (in x axis) if you want to use stroke (this is the trick for speed)
         * 2. If you use stroke, you need to test and adjust until you see pleasing results
         * 3. the sampler assumes it is being sampled from left to right in a rectangle, this is how it caches
         *    and finds the correct segment to compare if it is to the right/left of
         *
         *
         * @tparam number the number typeof points
         * @tparam rgba_ the rgba type
         * @tparam stroke_bits bits for stroke, play with it
         * @tparam precision_bits bits precision for points
         */
        template <typename number, typename rgba_=rgba_t<8,8,8,8>,
                unsigned stroke_bits=0, unsigned precision_bits=15>
        class d1_function_sampler {
        public:
            using rgba = rgba_;
            using rint= typename microgl::traits::conditional<precision_bits>=16,
                    microgl::ints::int64_t, microgl::ints::int32_t>::type;
            using vertex = microgl::vertex2<number>;
            using ivertex = microgl::vertex2<rint>;
            static constexpr rint ONE= rint(1)<<precision_bits;
            static constexpr rint stroke_band = 1u << stroke_bits;

            d1_function_sampler() = default;

            color_t color_stroke= {0, (1u<<rgba::g)-1,0, (1u<<rgba::a)-1};
            color_t color_fill= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, (1u<<rgba::a)-1};
            color_t color_background= {(1u<<rgba::r)-1, (1u<<rgba::g)-1, (1u<<rgba::b)-1, 0};

            void updatePoints(vertex * vertices, unsigned size) {
                points = vertices;
                _size=size;
            }

        private:

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

            unsigned _size=0;
            vertex *points;
            mutable rint latest_u=-1;
            mutable rint latest_index=0;
#define aaaa(x) ((x)<0?-(x):(x))

        public:
            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, precision_bits);
                const auto v_tag= convert(v, bits, precision_bits);
                rint dis=ONE;
                ivertex p{u_tag, v_tag},a, b;
                int ix = u_tag>=latest_u ? latest_index : 0;
                for (; ix < _size-1; ++ix) {
                    b.x = microgl::math::to_fixed(points[ix+1].x, precision_bits);
                    if(p.x<b.x) break;
                }
                latest_u=u_tag;
                latest_index=ix;

                if(ix==_size-1) {
                    ix = _size - 2;
                    b.x = microgl::math::to_fixed(points[ix+1].x, precision_bits);
                }

                a.x = microgl::math::to_fixed(points[ix].x, precision_bits);
                a.y = microgl::math::to_fixed(points[ix].y, precision_bits);
                b.y = microgl::math::to_fixed(points[ix+1].y, precision_bits);
                ivertex ab = b-a;
                rint signed_area= -(((p.x-a.x)*ab.y)>>precision_bits) + (((p.y-a.y)*ab.x)>>precision_bits);
                dis=signed_area;

                //
                rint dis_abs=aaaa(dis);
                output=color_background;
                rint aa_dis = (stroke_band - ((dis_abs)));
                if((dis)<=0)
                    output=color_fill;
                if (stroke_bits>0 && (aa_dis)>0) {
                    output.r = (dis_abs*rint(output.r) + aa_dis*rint(color_stroke.r)) >> stroke_bits;
                    output.g = (dis_abs*rint(output.g )+ aa_dis*rint(color_stroke.g)) >> stroke_bits;
                    output.b = (dis_abs*rint(output.b) + aa_dis*rint(color_stroke.b)) >> stroke_bits;
                    output.a = (dis_abs*rint(output.a) + aa_dis*rint(color_stroke.a)) >> stroke_bits;

                }

            }
#undef aaaa
        private:
        };

    }
}