#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>
#include <microgl/vec2.h>

namespace microgl {
    namespace sampling {

        /**
         * given a line, compute gradient in the [0,1]x[0,1] box.
         * experiment with line inside the [0,1]x[0,1] box in order to see pronounced results.
         * @tparam number
         * @tparam N
         */
        template <typename number, unsigned N=10>
        class radial_gradient : public sampler<radial_gradient<number, N>> {
            using base= sampler<radial_gradient<number, N>>;
            using l64= long long;
            using point_l64= vec2<l64>;
            using point= vec2<number>;
            static constexpr precision p_= precision::high;
            static constexpr precision_t p_bits= static_cast<precision_t>(p_);
            static constexpr precision_t p_bits_double= p_bits<<1;
            static constexpr l64 ONE= l64(1)<<p_bits;
            static constexpr l64 HALF= ONE>>1;

            struct stop_t {
                l64 where=0;
                l64 start_squared=0;
                l64 length_inverse=0;
                l64 length=0;
                color_t color{};
            };

            static inline
            l64 convert(l64 from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

            unsigned index= 0;
            stop_t _stops[N];
            l64 _cx=HALF, _cy=HALF, _radius=HALF;
        public:
            radial_gradient() : base{8, 8, 8, 8} {}
            radial_gradient(const number &cx, const number &cy, const number &radius) :
                    radial_gradient() {
                setNewRadial(cx, cy, radius);
            };

            void setNewRadial(const number &cx, const number &cy, const number &radius) {
                _cx= math::to_fixed(cx, p_bits);
                _cy= math::to_fixed(cy, p_bits);
                _radius= math::to_fixed(radius, p_bits);
                reset();
            }

            void addStop(const number & where, const color_t &color) {
                 l64 where_64= math::to_fixed(where, p_bits);
//                where_64= (where_64*where_64)>>p_bits;
                const l64 distance = (where_64*_radius)>>p_bits;
                auto & stop = _stops[index];
                stop.where= (where_64);
//                stop.where= (where_64*where_64)>>p_bits;
//                _radius= (_radius*_radius)>>p_bits;
//                stop.start_squared= distance;
                stop.start_squared= (distance * distance) >> p_bits;
                stop.color= color;
                if(index>0) {
//                    l64 l= ((_stops[index].where-_stops[index-1].where)*_radius)>>p_bits;
                    l64 a= (_stops[index].where*_radius)>>p_bits;
                    l64 b= (_stops[index-1].where*_radius)>>p_bits;
                    a = (a*a)>>p_bits;
                    b = (b*b)>>p_bits;
                    l64 l=a-b;
                    l64 l_inverse= (l64(1)<<p_bits_double)/l;
                    _stops[index].length_inverse= l_inverse;
//                    _stops[index].length_inverse= (l_inverse*l_inverse)>>p_bits;
                    _stops[index].length= l;
                }
                index++;
            }

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                const l64 dx= u_tag-_cx, dy= v_tag-_cy;
                const l64 distance_squared= ((dx * dx) >> p_bits) + ((dy * dy) >> p_bits);
                unsigned pos=0;
                const auto top= index;
                l64 distance_to_closest_stop= 0;
                for (pos=0; pos<top; ++pos) {
                    const l64 d= distance_squared - _stops[pos].start_squared;
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
                const l64 factor= (distance_to_closest_stop*l_inverse);
                output.r= l64(stop_0.color.r) + ((l64(stop_1.color.r-stop_0.color.r)*factor)>>p_bits_double);
                output.g= l64(stop_0.color.g) + ((l64(stop_1.color.g-stop_0.color.g)*factor)>>p_bits_double);
                output.b= l64(stop_0.color.b) + ((l64(stop_1.color.b-stop_0.color.b)*factor)>>p_bits_double);
                output.a= l64(stop_0.color.a) + ((l64(stop_1.color.a-stop_0.color.a)*factor)>>p_bits_double);
            }

            /*
            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                const auto u_tag= convert(u, bits, p_bits);
                const auto v_tag= convert(v, bits, p_bits);
                unsigned pos=0;
                const auto top= index;
                l64 distance= 0;
                for (pos=0; pos<top; ++pos) {
                    const l64 d= _stops[pos].line.distance(u_tag, v_tag);
                    if(d<0) break;
                    distance=d;
                }
                if(pos==top) {
                    output=_stops[top-1].color; return;
                } else if(pos==0) {
                    output=_stops[0].color; return;
                }
                const auto & stop_0= _stops[pos-1];
                const auto & stop_1= _stops[pos];
                const auto & l_inverse= _stops[pos].length_inverse;
                const l64 factor= distance*l_inverse;
                output.r= l64(stop_0.color.r) + ((l64(stop_1.color.r-stop_0.color.r)*factor)>>p_bits_double);
                output.g= l64(stop_0.color.g) + ((l64(stop_1.color.g-stop_0.color.g)*factor)>>p_bits_double);
                output.b= l64(stop_0.color.b) + ((l64(stop_1.color.b-stop_0.color.b)*factor)>>p_bits_double);
                output.a= l64(stop_0.color.a) + ((l64(stop_1.color.a-stop_0.color.a)*factor)>>p_bits_double);
            }
             */

            void reset() {
                index=0;
            }

        private:

        };

    }
}