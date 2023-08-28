#pragma once
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

#include "../math.h"
#include "precision.h"

namespace microgl {
    namespace sampling {

        /**
         * extract the rgba info of a sampler
         */
        template<class sampler>
        using sampler_rgba = typename sampler::rgba;

        /**
         * sample uv coords from sampler with a different number system.
         * This is handy because samplers work in quantized space to be more
         * efficient and sometimes it is not convenient.
         *
         * @tparam Sampler the sampler type
         * @tparam number the number type
         *
         * @param sampler the sampler reference
         * @param u the u coord
         * @param v the v coord
         * @param p precision can be {low, medium, high, amazing}
         * @param output output color
         */
        template<class Sampler, typename number>
        inline void sample(const Sampler & sampler,
                           const number &u, const number &v,
                           const enum microgl::precision &p,
                           color_t &output) {
            auto bits = static_cast<precision_t>(p);
            const int u_fixed = microgl::math::to_fixed(u, bits);
            const int v_fixed = microgl::math::to_fixed(v, bits);
            sampler.sample(u_fixed, v_fixed, bits, output);
        }

        /**
         * a base sampler container, includes a utility methods and crpt
         * routing for compile time polymorphism
         *
         * @tparam rgba_ the rgba_t info type
         * @tparam impl the type of the derived class
         */
        template<typename rgba_, typename impl>
        class base_sampler : public microgl::traits::crpt<impl> {
        protected:
        public:
            using rgba = rgba_;

            base_sampler()= default;

            inline void sample(const int u, const int v,
                               const microgl::ints::uint8_t bits, color_t &output) const {
                this->derived().sample(u, v, bits, output);
            }

            template<typename number>
            inline void sample(const number &u, const number &v,
                               const enum microgl::precision &p,
                               color_t &output) {
                auto bits = static_cast<precision_t>(p);
                const int u_fixed = microgl::math::to_fixed(u, bits);
                const int v_fixed = microgl::math::to_fixed(v, bits);
                this->derived().sample(u_fixed, v_fixed, bits, output);
            }

        };

    }
}