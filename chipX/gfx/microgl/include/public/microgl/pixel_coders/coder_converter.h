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
                        to::r, to::g,
                        to::b, to::a> {
        private:
            using base = coder_rgba<from, to::r,
                                    to::g, to::b,
                                    to::a>;
        public:
            using base::decode;
            using base::encode;
            coder_converter() : base{} {};
        };
    }
}