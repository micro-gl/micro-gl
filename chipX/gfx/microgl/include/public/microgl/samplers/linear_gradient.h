#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>

namespace microgl {
    namespace sampling {

        template <bool horizontal=true, unsigned N=10>
        class linear_gradient : public sampler<linear_gradient<horizontal, N>> {
            using base= sampler<linear_gradient<horizontal, N>>;
            static constexpr precision p= precision::high;
            static constexpr precision_t p_bits= static_cast<precision_t>(p);
            static constexpr precision_t p_bits_double= p_bits<<1;
            using l64= long long;

            static inline
            l64 convert(l64 from_value,
                        unsigned from_precision,
                        unsigned to_precision) {
                if(from_precision==to_precision)
                    return from_value;
                else if(from_precision>to_precision)
                    return (from_value)>>(from_precision - to_precision);
                else
                    return (from_value)<<(to_precision - from_precision);
            }

        public:
            struct stop_t {
                l64 where;
                l64 length_inverse;
                color_t color;
            };

            linear_gradient() : base{8,8,8,8} {};

            template <typename number>
            void addStop(const number & where, const color_t &color) {
                _stops[index] = {math::to_fixed(where, p_bits), 0, color};
                if(index>0) {
                    l64 l= _stops[index].where-_stops[index-1].where;
                    l64 l_inverse= (l64(1)<<p_bits_double)/l;
                    _stops[index].length_inverse= l_inverse;
                }
                index++;
            }

            void reset() {
                index=0;
            }

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                const auto u_tag= convert(horizontal ? u : v, bits, p_bits);
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
                const l64 factor= (u_tag-stop_0.where)*l_inverse;
                output.r= l64(stop_0.color.r) + ((l64(stop_1.color.r-stop_0.color.r)*factor)>>p_bits_double);
                output.g= l64(stop_0.color.g) + ((l64(stop_1.color.g-stop_0.color.g)*factor)>>p_bits_double);
                output.b= l64(stop_0.color.b) + ((l64(stop_1.color.b-stop_0.color.b)*factor)>>p_bits_double);
                output.a= l64(stop_0.color.a) + ((l64(stop_1.color.a-stop_0.color.a)*factor)>>p_bits_double);
            }


        private:

            unsigned index= 0;
            stop_t _stops[N];
        };

    }
}