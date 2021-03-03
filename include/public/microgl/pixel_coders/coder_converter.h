#pragma once

#include <microgl/pixel_coders/coder_rgba.h>

namespace microgl {
    namespace coder {

        /**
         * a pixel coder that converts from_sampler one coder to another
         *
         * @tparam from_sampler
         * @tparam to
         */
        template<class from_sampler, class to_sampler>
        class coder_converter :
                        public coder_rgba<from_sampler, typename to_sampler::rgba> {
        private:
            using base = coder_rgba<from_sampler, typename to_sampler::rgba>;

        public:
            using base::decode;
            using base::encode;
            coder_converter() : base{} {};
        };
    }
}