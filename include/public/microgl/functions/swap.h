#pragma once

namespace microgl {

    namespace functions {
        template<typename T>
        inline void swap(T &a, T &b) {
            T temp = a;
            a = b;
            b = temp;
        }
    }
}
