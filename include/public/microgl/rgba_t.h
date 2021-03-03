#pragma once

#include <cstdint>

namespace microgl {
/**
 * generic rgba_t container
 *
 * @tparam R
 * @tparam G
 * @tparam B
 * @tparam A
 */
    template<uint8_t R, uint8_t G, uint8_t B, uint8_t A>
    struct rgba_t {
        static constexpr uint8_t r = R;
        static constexpr uint8_t g = G;
        static constexpr uint8_t b = B;
        static constexpr uint8_t a = A;

        rgba_t() = delete;
        ~rgba_t() = delete;
    };
}
