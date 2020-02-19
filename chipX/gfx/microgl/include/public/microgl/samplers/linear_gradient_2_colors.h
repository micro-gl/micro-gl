#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>

namespace microgl {
    namespace sampling {

        template <bool horizontal=true>
        class linear_gradient_2_colors : public sampler<linear_gradient_2_colors<horizontal>> {
            using base= sampler<linear_gradient_2_colors<horizontal>>;
            using l64= long long;

        public:
            linear_gradient_2_colors() : linear_gradient_2_colors({0,0,0,0}, {0,0,0,0}) {}
            linear_gradient_2_colors(const color_t& color_1, const color_t& color_2) :
                    base{8,8,8,8}, color1{color_1}, color2{color_2} {};

            color_t color1= {255,0,0};
            color_t color2= {0,0,255};

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                const auto t= horizontal ? u : v;
                output.r= l64(color1.r) + ((l64(color2.r-color1.r)*t)>>bits);
                output.g= l64(color1.g) + ((l64(color2.g-color1.g)*t)>>bits);
                output.b= l64(color1.b) + ((l64(color2.b-color1.b)*t)>>bits);
                output.a= l64(color1.a) + ((l64(color2.a-color1.a)*t)>>bits);
            }

        private:
        };

    }
}