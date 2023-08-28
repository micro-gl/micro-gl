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

        template <unsigned degree=90, typename rgba_=rgba_t<8,8,8,0>, bool useBigIntegers=false>
        struct linear_gradient_2_colors {
            using rgba = rgba_;
            using rint= typename microgl::traits::conditional<useBigIntegers, microgl::ints::int64_t, microgl::ints::int32_t>::type;

        public:
            linear_gradient_2_colors() : linear_gradient_2_colors({0,0,0,0}, {0,0,0,0}) {}
            linear_gradient_2_colors(const color_t& color_1, const color_t& color_2) :
                    color1{color_1}, color2{color_2} {};

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
                else if(degree<=315) t=((u-v)>>1)+h;
                output.r= rint(color1.r) + ((rint(color2.r-color1.r)*t)>>bits);
                output.g= rint(color1.g) + ((rint(color2.g-color1.g)*t)>>bits);
                output.b= rint(color1.b) + ((rint(color2.b-color1.b)*t)>>bits);
                output.a= rint(color1.a) + ((rint(color2.a-color1.a)*t)>>bits);
            }

        private:
        };

    }
}