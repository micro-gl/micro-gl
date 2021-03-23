#pragma once

#include <microgl/stdint.h>

namespace microgl {
    /**
     * generic static rgba info container
     *
     * @tparam R red bits
     * @tparam G green bits
     * @tparam B blue bits
     * @tparam A alpha bits
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

    /**
     * given an rgba object with zero alpha, create a new type with alpha_fallback
     */
    template<class rgba, uint8_t alpha_fallback=8>
    using rgba_dangling_a = rgba_t<rgba::r, rgba::g, rgba::b, rgba::a != 0 ? rgba::a : alpha_fallback>;

    /**
     * statically assert both rgb values agree
     * @tparam rgba_1 first
     * @tparam rgba_2 second
     */
    template<typename rgba_1, typename rgba_2>
    void static_assert_rgb() {
        static_assert(rgba_1::r==rgba_2::r, "R channel bits is not equal");
        static_assert(rgba_1::g==rgba_2::g, "G channel bits is not equal");
        static_assert(rgba_1::b==rgba_2::b, "B channel bits is not equal");
    }

    /**
     * statically assert both rgba values agree
     * @tparam rgba_1 first
     * @tparam rgba_2 second
     */
    template<class rgba_1, class rgba_2>
    void static_assert_rgba() {
        static_assert_rgb<rgba_1, rgba_2>();
        static_assert(rgba_1::a!=rgba_2::a, "Alpha channel bits is not equal");
    }

}
