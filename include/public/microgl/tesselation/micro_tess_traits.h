#pragma once

namespace microtess {
    namespace traits {
        template<class T1, class T2>
        struct is_same {
            const static bool value = false;
        };
        template<class T>
        struct is_same<T, T> {
            const static bool value = true;
        };

        template<class number>
        constexpr bool is_float_point() {
            return is_same<float, number>::value ||
                   is_same<double, number>::value ||
                   is_same<long double, number>::value;
        }

        template<bool B, class T, class F>
        struct conditional {
            typedef T type;
        };
        template<class T, class F>
        struct conditional<false, T, F> {
            typedef F type;
        };

        template<bool, class _Tp = void> struct enable_if {};
        template<class _Tp> struct enable_if<true, _Tp> { typedef _Tp type; };

    }
}