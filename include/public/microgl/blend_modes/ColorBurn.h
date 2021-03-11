#pragma once

namespace microgl {
    namespace blendmode {

        class ColorBurn {
        private:

            template<uint8_t bits>
            static inline
            uint blend_ColorBurn(cuint b, cuint s) {
                constexpr cuint max = (uint(1) << bits) - 1;
                if(s==0) return s;
                cuint bb = max - ((max - b)*max) / s;
                return bb<0 ? 0 : bb;
            }

        public:

            template<uint8_t R, uint8_t G, uint8_t B>
            static inline void blend(const color_t &b,
                                     const color_t &s,
                                     color_t &output) {

                output.r = blend_ColorBurn<R>(b.r, s.r);
                output.g = blend_ColorBurn<G>(b.g, s.g);
                output.b = blend_ColorBurn<B>(b.b, s.b);
            }

        };

    }
}