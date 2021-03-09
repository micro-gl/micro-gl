#pragma once

#include <microgl/pixel_coders/coder_converter_rgba.h>

namespace microgl {
    namespace coder {

        /**
         * a pixel coder that converts from_sampler one coder to another
         *
         * @tparam from_coder
         * @tparam to
         *
         */
        template<class from_coder, class to_coder>
        using coder_converter = coder_converter_rgba<from_coder, typename to_coder::rgba>;

    }
}