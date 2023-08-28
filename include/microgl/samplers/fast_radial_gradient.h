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
#include <microgl/color.h>
#include <microgl/traits.h>

namespace microgl {
    namespace sampling {

        /**
         * given a line, compute a fast radial gradient in the [0,1]x[0,1] box.
         * this is not a classic linear radial gradient, this would require computing
         * a sqrt function which I avoid, therefore my interpolation function is closer
         * to a quadratic function interpolation to calculate interpolation factor
         */
        template <typename number, unsigned N=10, typename rgba_=rgba_t<8,8,8,0>,
                  enum precision $precision=precision::medium>
        struct fast_radial_gradient {
            using rgba = rgba_;
        private:
            using rint_big=microgl::ints::int64_t;
            static constexpr precision_t p_bits= static_cast<precision_t>($precision);
            using rint= typename microgl::traits::conditional<p_bits>=16, microgl::ints::int64_t, microgl::ints::int32_t>::type;
            static constexpr precision_t p_bits_double= p_bits<<1;
            static constexpr rint ONE= rint(1)<<p_bits;
            static constexpr rint HALF= ONE>>1;

            struct stop_t {
                rint where=0;
                rint start_squared=0;
                rint length_inverse=0;
                rint length=0;
                color_t color{};
            };

            static inline
            rint convert(rint from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

            unsigned index= 0;
            stop_t _stops[N];
            rint _cx=HALF, _cy=HALF, _radius=HALF;
        public:
            fast_radial_gradient()=default;
            fast_radial_gradient(const number &cx, const number &cy, const number &radius) :
                    fast_radial_gradient() {
                setNewRadial(cx, cy, radius);
            };

            void setNewRadial(const number &cx, const number &cy, const number &radius) {
                _cx= math::to_fixed(cx, p_bits);
                _cy= math::to_fixed(cy, p_bits);
                _radius= math::to_fixed(radius, p_bits);
                reset();
            }

            void addStop(const number & where, const color_t &color) {
                rint where_64= math::to_fixed(where, p_bits);
                const rint distance = (rint_big(where_64)*_radius)>>p_bits; // p_bits number
                auto & stop = _stops[index];
                stop.where= (where_64);
                stop.start_squared= (distance * distance) >> p_bits;
                stop.color= color;
                if(index>0) {
                    rint a_= (rint_big(_stops[index].where)*_radius)>>p_bits;
                    rint b_= (rint_big(_stops[index-1].where)*_radius)>>p_bits;
                    a_ = (a_*a_)>>p_bits;
                    b_ = (b_*b_)>>p_bits;
                    rint l=a_-b_;
                    rint l_inverse= (rint_big(1)<<p_bits_double)/l;
                    _stops[index].length_inverse= l_inverse;
                    _stops[index].length= l;
                }
                index++;
            }

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                // note:: inside the critical sampling we use only one type of integer, to allow for 32
                //        bit computers for example to be efficient and avoid double registers
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                const rint dx= u_tag-_cx, dy= v_tag-_cy;
                const rint distance_squared= ((dx * dx) >> p_bits) + ((dy * dy) >> p_bits);
                unsigned pos=0;
                const auto top= index;
                rint distance_to_closest_stop= 0;
                for (pos=0; pos<top; ++pos) {
                    const rint d= distance_squared - _stops[pos].start_squared;
                    if(d<0) break;
                    distance_to_closest_stop=d;
                }
                if(pos==top) {
                    output=_stops[top-1].color; return;
                } else if(pos==0) {
                    output=_stops[0].color; return;
                }
                const auto & stop_0= _stops[pos-1];
                const auto & stop_1= _stops[pos];
                const auto & l_inverse= _stops[pos].length_inverse;
                const rint factor= (distance_to_closest_stop*l_inverse)>>p_bits;
                output.r= rint(stop_0.color.r) + ((rint(stop_1.color.r-stop_0.color.r)*factor)>>p_bits);
                output.g= rint(stop_0.color.g) + ((rint(stop_1.color.g-stop_0.color.g)*factor)>>p_bits);
                output.b= rint(stop_0.color.b) + ((rint(stop_1.color.b-stop_0.color.b)*factor)>>p_bits);
                output.a= rint(stop_0.color.a) + ((rint(stop_1.color.a-stop_0.color.a)*factor)>>p_bits);
            }

            void reset() {
                index=0;
            }

        private:

        };

    }
}