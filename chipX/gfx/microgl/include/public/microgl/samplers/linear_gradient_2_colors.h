#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/sampler.h>

namespace microgl {
    namespace sampling {

        template <unsigned degree=90, uint8_t r=8, uint8_t g=8, uint8_t b=8, uint8_t a=0, bool useBigIntegers=false>
        class linear_gradient_2_colors : public sampler<r,g,b,a, linear_gradient_2_colors<degree, r,g,b,a, useBigIntegers>> {
            using base= sampler<r,g,b,a, linear_gradient_2_colors<degree, r,g,b,a, useBigIntegers>>;
            using rint_big=int64_t;
            using rint= typename microgl::traits::conditional<useBigIntegers, int64_t, int32_t>::type;

        public:
            linear_gradient_2_colors() : linear_gradient_2_colors({0,0,0,0}, {0,0,0,0}) {}
            linear_gradient_2_colors(const color_t& color_1, const color_t& color_2) :
                    base{}, color1{color_1}, color2{color_2} {};

            color_t color1= {255,0,0};
            color_t color2= {0,0,255};

            inline void sample(const int u, const int v,
                               const unsigned bits, color_t &output) const {
                rint t=0, h= rint(1)<<(bits-1);
                if(degree<=0 || degree>315) t=u;
                else if(degree<=45) t=(u+v)>>1;
                else if(degree<=90) t=v;
                else if(degree<=135) t=(rint(1)<<bits)-(((u-v)>>1)+h);
                else if(degree<=180) t=(rint(1)<<bits)-u;
                else if(degree<=225) t=(rint(1)<<bits)-((u+v)>>1);
                else if(degree<=270) t=(rint(1)<<bits)-v;
                else if(degree<=315) t=((u-v)>>1)-h;
                output.r= rint(color1.r) + ((rint(color2.r-color1.r)*t)>>bits);
                output.g= rint(color1.g) + ((rint(color2.g-color1.g)*t)>>bits);
                output.b= rint(color1.b) + ((rint(color2.b-color1.b)*t)>>bits);
                output.a= rint(color1.a) + ((rint(color2.a-color1.a)*t)>>bits);
            }

        private:
        };

    }
}