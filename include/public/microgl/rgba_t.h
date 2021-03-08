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
        rgba_t(const rgba_t &) = delete;
        rgba_t& operator=(const rgba_t &) = delete;
        ~rgba_t() = delete;
    };

    template<typename rgba_1, typename rgba_2>
    void static_assert_rgb() {
        static_assert(rgba_1::r==rgba_2::r, "R channel bits is not equal");
        static_assert(rgba_1::g==rgba_2::g, "G channel bits is not equal");
        static_assert(rgba_1::b==rgba_2::b, "B channel bits is not equal");
    }

    template<class rgba_1, class rgba_2>
    void static_assert_rgba() {
        static_assert_rgb<rgba_1, rgba_2>();
        static_assert(rgba_1::a!=rgba_2::a, "Alpha channel bits is not equal");
    }

}
