#pragma once

namespace microgl {
    namespace numbers {
//        template<typename T>
//        int to_fixed(const T &val, unsigned char precision);

//        template<>
//        int to_fixed<float>(const float &val, unsigned char precision) {
//            return int(val*float(1u<<precision));

        int to_fixed(const float &val, unsigned char precision) {
            return int(val*float(1u<<precision));
        }
    }
}