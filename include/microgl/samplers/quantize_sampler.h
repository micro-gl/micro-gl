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

namespace microgl {
    namespace sampling {

        /**
         * This sampler can lower the sampling resolution of another sampler, thus creating a
         * very cool old-school effect
         *
         * @tparam Sampler the sampler type we want to quantize
         */
        template <typename Sampler>
        class quantize_sampler {
            Sampler * _sampler = nullptr;
            using uchar = unsigned char;
        public:
            using rgba = typename Sampler::rgba;

            // the bits to transform into
            int q_bits=5;

            /**
             *
             * @param sampler sampler pointer
             * @param q_bits the bits we want to crush into
             */
            explicit quantize_sampler(Sampler * sampler, uchar q_bits) : _sampler{sampler}, q_bits{q_bits} {
            }

        private:
            static inline
            int convert(const int from_value, const uchar from_precision, const uchar to_precision) {
                const int pp= int(from_precision) - int(to_precision);
                if(pp==0) return from_value;
                else if(pp > 0) return from_value>>pp;
                else return from_value<<(-pp);
            }

        public:
            inline void sample(const int u, const int v,
                               const unsigned bits,
                               color_t &output) const {
                const auto u_tag= convert(u, bits, q_bits);
                const auto v_tag= convert(v, bits, q_bits);
                _sampler->sample(u_tag, v_tag, q_bits, output);
            }

        };

    }
}