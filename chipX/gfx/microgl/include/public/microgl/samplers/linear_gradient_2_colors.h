#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>

namespace microgl {
    namespace sampling {

        template <unsigned degree=90>
        class linear_gradient_2_colors : public sampler<linear_gradient_2_colors<degree>> {
            using base= sampler<linear_gradient_2_colors<degree>>;
            using l64= long long;

        public:
            linear_gradient_2_colors() : linear_gradient_2_colors({0,0,0,0}, {0,0,0,0}) {}
            linear_gradient_2_colors(const color_t& color_1, const color_t& color_2) :
                    base{8,8,8,8}, color1{color_1}, color2{color_2} {};

            color_t color1= {255,0,0};
            color_t color2= {0,0,255};

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                l64 t=0, h= l64(1)<<(bits-1);
                if(degree<=0 || degree>315) t=u;
                else if(degree<=45) t=(u+v)>>1;
                else if(degree<=90) t=v;
                else if(degree<=135) t=(1<<bits)-(((u-v)>>1)+h);
                else if(degree<=180) t=(1<<bits)-u;
                else if(degree<=225) t=(1<<bits)-((u+v)>>1);
                else if(degree<=270) t=(1<<bits)-v;
                else if(degree<=315) t=((u-v)>>1)-h;
                output.r= l64(color1.r) + ((l64(color2.r-color1.r)*t)>>bits);
                output.g= l64(color1.g) + ((l64(color2.g-color1.g)*t)>>bits);
                output.b= l64(color1.b) + ((l64(color2.b-color1.b)*t)>>bits);
                output.a= l64(color1.a) + ((l64(color2.a-color1.a)*t)>>bits);
            }

        private:
        };

    }
}