#pragma once

namespace microgl {

    namespace functions {

        template<typename T>
        inline T clamp(const T &val, const T &e0, const T &e1) {
            return val<e0 ? e0 : val>e1 ? e1 : val;
        }
    }
}
