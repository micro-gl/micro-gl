#pragma once

#include <microgl/lut.h>

namespace microgl {
    namespace lut {

        /**
         * dynamic runtime (stack or heap storage) lookup table generation
         * @tparam bits1
         * @tparam bits2
         */
        template <uint8_t bits1, uint8_t bits2, bool heap=false>
        struct dynamic_lut_bits {
        private:
            // let's fail if more than 8 bits
            typename microgl::traits::enable_if<bits1<=8 && bits2<=8, bool>::type fail_if_more_than_8_bits;
            constexpr static unsigned size=1u<<bits1;
            dynamic_lut<uint8_t, size, heap> lut;

        public:
            dynamic_lut_bits() : lut{} {
                const auto lambda = [](const int index) {
                    return bits1==0 ? 0 : (index*((int(1)<<bits2)-1))/((int(1)<<bits1)-1);
                };
                lut.generate(lambda);
            };

            uint8_t operator[](const unsigned index) const {
                return lut[index];
            }
        };

        /**
         * compile time (static storage) lookup table generation
         * @tparam bits1
         * @tparam bits2
         */
        template <uint8_t bits1, uint8_t bits2>
        struct static_lut_bits {
        private:
            // let's fail if more than 8 bits
            typename microgl::traits::enable_if<bits1<=8 && bits2<=8, bool>::type fail_if_more_than_8_bits;
            constexpr static unsigned size=1u<<bits1;

            struct func {
                // this is the function to generate the LUT elements
                static constexpr uint8_t apply(int n) {
                    // one liner to support C++11 restrictive constexpr function
                    return bits1==0 ? 0 : (n*((int(1)<<bits2)-1))/((int(1)<<bits1)-1);
                }
            };
            const static static_lut<uint8_t, size, func> lut;

        public:
            static_lut_bits()= delete;
            static uint8_t get(const int & n) {
                return lut.get(n);
            }
        };

        // definition
        template <uint8_t bits1, uint8_t bits2>
        const static_lut<uint8_t, static_lut_bits<bits1, bits2>::size, typename static_lut_bits<bits1, bits2>::func> static_lut_bits<bits1, bits2>::lut;

    }

}
