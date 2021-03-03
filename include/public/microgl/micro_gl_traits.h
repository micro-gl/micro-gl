#pragma once

#include <cstdint>

namespace microgl {
    namespace traits {

        template<class T1, class T2>
        struct is_same { const static bool value = false; };
        template<class T>
        struct is_same<T, T> { const static bool value = true; };

        template<class number>
        constexpr bool is_float_point() {
            return is_same<float, number>::value || is_same<double, number>::value || is_same<long double , number>::value;
        }

        template<bool B, class T, class F>
        struct conditional { typedef T type; };
        template<class T, class F>
        struct conditional<false, T, F> { typedef F type; };

        template <bool, class _Tp = void> struct enable_if {};
        template <class _Tp> struct enable_if<true, _Tp> {typedef _Tp type;};

        template<uint8_t BITS>
        struct unsigned_type_infer {
            static constexpr uint8_t bits = BITS;
            using type = typename microgl::traits::conditional<
                    BITS<=8, uint8_t ,
                    typename microgl::traits::conditional<
                            BITS<=16, uint16_t ,
                            typename microgl::traits::conditional<
                                    BITS<=32, uint32_t , uint64_t >::type>::type>::type;
        };

        template<uint8_t BITS>
        struct signed_type_infer {
            static constexpr uint8_t bits = BITS;
            using type = typename microgl::traits::conditional<
                    BITS<=8, int8_t ,
                    typename microgl::traits::conditional<
                            BITS<=16, int16_t ,
                            typename microgl::traits::conditional<
                                    BITS<=32, int32_t , int64_t >::type>::type>::type;
        };

    }

}