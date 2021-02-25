#pragma once

#include <microgl/pixel_coders/coder_rgba.h>

namespace microgl {
    namespace coder {

        /**
         * a pixel coder that converts from one coder to another
         *
         * @tparam from
         * @tparam to
         */
        template<class from, class to>
        class coder_converter :
                public coder_rgba<from,
                        to::red_bits(), to::green_bits(),
                        to::blue_bits(), to::alpha_bits()> {
        private:
            using base = coder_rgba<from, to::red_bits(),
                                    to::green_bits(), to::blue_bits(),
                                    to::alpha_bits()>;
        public:
            using base::decode;
            using base::encode;
            coder_converter() : base{} {};
        };
    }
}