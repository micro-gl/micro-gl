#pragma once

namespace microgl {
    namespace traits {
        template<class T1, class T2>
        struct is_same {
        public:
            const static bool value = false;
        };

        template<class T>
        struct is_same<T, T> {
        public:
            const static bool value = true;
        };

        template<class number>
        constexpr bool is_float_point() {
            return is_same<float, number>::value || is_same<double, number>::value || is_same<long double , number>::value;
        }

    }

}