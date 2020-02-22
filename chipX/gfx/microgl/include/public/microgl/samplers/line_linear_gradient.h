#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/math.h>
#include <microgl/sampler.h>
#include <microgl/vec2.h>

namespace microgl {
    namespace sampling {

        template <typename number, unsigned N=10>
        class line_linear_gradient : public sampler<line_linear_gradient<number, N>> {
            using base= sampler<line_linear_gradient<number, N>>;
            static constexpr precision p_= precision::high;
            static constexpr precision_t p_bits= static_cast<precision_t>(p_);
            static constexpr precision_t p_bits_double= p_bits<<1;
            using l64= long long;
            using point_l64= vec2<l64>;
            using point= vec2<number>;

            // ax + by + c = 0
            struct line_t {
                void updateLine(const point_l64 &p, const point_l64 & n) {
                    a=n.x, b=n.y, c= -((n*p)); // a,b are in P bits,but c is in 2P bits
                    inv_normal_length = (1<<p_bits_double) / microgl::math::sqrt(n.x*n.x + n.y*n.y);
                }

                // dist(ax + by + c = 0, (x0, y0)) = (a*x0 + b*y0 + c)/sqrt(a^2 + b^2)
                l64 distance(const l64 &x, const l64 &y) const {
                    return (((a*x) + b*y + c)*inv_normal_length)>>p_bits_double;
                }

                bool leftOf(const l64 &x, const l64 &y) const {
                    return (a*x + b*y + c)<0;
                }

                l64 inv_normal_length=0;
                l64 a=0, b=0, c=0;
            };

            struct stop_t {
                l64 where=0;
                l64 length_inverse=0;
                line_t line{};
                color_t color{};
            };

            static inline
            l64 convert(l64 from_value, int from_precision, int to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

            point_l64 _start{}, _end{}, _direction{};
            l64 _length;
            unsigned index= 0;
            stop_t _stops[N];

        public:

            line_linear_gradient(const vec2<number> & start, const vec2<number> & end) :
                    base{8, 8, 8, 8} {
                const auto dir= end-start;
                const auto length= math::length(dir.x, dir.y);

#define f math::to_fixed
                _start= {f(start.x, p_bits), f(start.y, p_bits)};
                _end= {f(end.x, p_bits), f(end.y, p_bits)};
                _direction= {f(dir.x, p_bits), f(dir.y, p_bits)};
                _length= f(length, p_bits);
#undef f
            };

            void addStop(const number & where, const color_t &color) {
                const l64 where_64= math::to_fixed(where, p_bits);
                const auto p_64= _start+((_direction*where_64)>>p_bits);
                auto & stop = _stops[index];

                stop.line.updateLine(p_64, _direction);
                stop.where= where_64;
                stop.color= color;

                if(index>0) {
                    l64 l= ((_stops[index].where-_stops[index-1].where)*_length)>>p_bits;
                    l64 l_inverse= (l64(1)<<p_bits_double)/l;
                    _stops[index].length_inverse= l_inverse;
                }
                index++;
            }

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

            void reset() {
                index=0;
            }

        private:

        };

    }
}