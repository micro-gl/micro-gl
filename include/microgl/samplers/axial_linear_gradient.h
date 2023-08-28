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

namespace microgl {
    namespace sampling {

        template <unsigned degree=0, unsigned N=10, typename rgba_=rgba_t<8,8,8,0>,
                  enum precision $precision=precision::medium>
        struct axial_linear_gradient {
            using rgba = rgba_;

        private:
            static constexpr precision_t p_bits= static_cast<precision_t>($precision);
            static constexpr precision_t p_bits_double= p_bits<<1;
            using rint_big=microgl::ints::int64_t;
            using rint= typename microgl::traits::conditional<p_bits>=16, microgl::ints::int64_t, microgl::ints::int32_t>::type;

            struct stop_t {
                rint where=0;
                rint length_inverse=0;
                color_t color{};
            };

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

        public:

            template <typename number>
            void addStop(const number & where, const color_t &color) {
                auto & stop= _stops[index];
                stop.where= math::to_fixed(where, p_bits);
                stop.color= color;
                if(index>0) {
                    rint l= _stops[index].where-_stops[index-1].where;
                    rint l_inverse= (rint_big (1)<<p_bits_double)/l;
                    _stops[index].length_inverse= l_inverse;
                }
                index++;
            }

            void reset() {
                index=0;
            }

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                rint t, h= rint(1)<<(bits-1);
                if(degree<=0 || degree>315) t=u;
                else if(degree<=45) t=(u+v-h);
                else if(degree<=90) t=v;
                else if(degree<=135) t=(rint(1)<<bits)-(u-v+h);
                else if(degree<=180) t=(rint(1)<<bits)-u;
                else if(degree<=225) t=(rint(1)<<bits)-(u+v-h);
                else if(degree<=270) t=(rint(1)<<bits)-v;
                else if(degree<=315) t=u-v+h;
                const auto u_tag= convert(t, bits, p_bits);
                unsigned pos=0;
                for (pos = 0; pos<index && u_tag>=_stops[pos].where; ++pos);
                if(pos==index) {
                    output=_stops[index-1].color;
                    return;
                } else if(pos==0) {
                    output=_stops[0].color;
                    return;
                }
                const auto & stop_0= _stops[pos-1];
                const auto & stop_1= _stops[pos];
                const auto & l_inverse= _stops[pos].length_inverse;
                // axis aligned gradients are faster because (u_tag-stop_0.where) is the stop
                // distance function and involves only subtraction
                const rint factor= ((u_tag-stop_0.where)*l_inverse)>>p_bits;
                output.r= rint(stop_0.color.r) + ((rint(stop_1.color.r-stop_0.color.r)*factor)>>p_bits);
                output.g= rint(stop_0.color.g) + ((rint(stop_1.color.g-stop_0.color.g)*factor)>>p_bits);
                output.b= rint(stop_0.color.b) + ((rint(stop_1.color.b-stop_0.color.b)*factor)>>p_bits);
                output.a= rint(stop_0.color.a) + ((rint(stop_1.color.a-stop_0.color.a)*factor)>>p_bits);
            }

        private:

            unsigned index= 0;
            stop_t _stops[N];
        };

    }
}