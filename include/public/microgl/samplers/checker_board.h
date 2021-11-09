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

#include <microgl/samplers/sampler.h>

namespace microgl {
    namespace sampling {

        /**
         * checker board pattern sampler
         *
         * @tparam rgba_ the rgba info
         */
        template<typename rgba_=rgba_t<8,8,8,0>>
        struct checker_board {
            using rgba = rgba_;

            /**
             * ctor
             * @param color1 1st color
             * @param color2 2nd color
             * @param horizontal horizontal splits count
             * @param vertical vertical split count
             */
            explicit checker_board(const color_t & color1,
                                   const color_t & color2,
                                   unsigned horizontal=2,
                                   unsigned vertical=2) : color1{color1}, color2{color2},
                                                          horizontal{horizontal}, vertical{vertical} {

            }

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                auto u_ = (horizontal*u)>>(bits);
                auto v_ = (vertical*v)>>(bits);

                // (u_ & 1)==1 if odd and ==0 if even
                if((u_ & 1)==(v_ & 1))
                    output = color1;
                else output = color2;
            }

            color_t color1, color2;
            unsigned horizontal, vertical;
        };

        /**
         * checker board pattern sampler with power of 2 splits. this should be
         * faster because no multiplication operation occurs, only bit shifting.
         *
         * @tparam H_POW exponent power for horizontal split count
         * @tparam V_POW exponent power for vertical split count
         * @tparam rgba_ rgba info
         */
        template<unsigned H_POW, unsigned V_POW, typename rgba_=rgba_t<8,8,8,0>>
        struct checker_board_pot {
            using rgba = rgba_;

            color_t color1, color2;

            /**
             * ctor
             * @param color1 1st color
             * @param color2 2nd color
             */
            explicit checker_board_pot(const color_t & color1,
                                   const color_t & color2) : color1{color1}, color2{color2} {
            }

            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                auto u_ = (u<<H_POW)>>(bits);
                auto v_ = (v<<V_POW)>>(bits);

                // (u_ & 1)==1 if odd and ==0 if even
                if((u_ & 1)==(v_ & 1))
                    output = color1;
                else output = color2;
            }

        };

    }
}