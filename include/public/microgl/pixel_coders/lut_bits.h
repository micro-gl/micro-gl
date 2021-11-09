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

#include "lut.h"
#include "../stdint.h"

namespace microgl {
    namespace lut {
        /**
         * dynamic runtime (stack or heap storage) lookup table generation
         * @tparam bits1 from bits
         * @tparam bits2 to bits
         * @tparam heap create on heap or stack
         * @tparam mute mute, no data is spent
         */
        template <uint8_t bits1, uint8_t bits2, bool heap=false, bool mute=false>
        struct dynamic_lut_bits {
        private:
            // let's fail if more than 8 bits
            typename microgl::traits::enable_if<bits1<=8 && bits2<=8, bool>::type fail_if_more_than_8_bits;
            constexpr static unsigned size=1u<<bits1;
            dynamic_lut<uint8_t, size, heap, mute> lut;

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
