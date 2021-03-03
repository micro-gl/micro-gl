#pragma once

namespace microgl {

    namespace functions {

        template<typename number>
        inline number clamp(const number &val, const number &e0, const number &e1) {
            return val<e0 ? e0 : val>e1 ? e1 : val;
        }
    }
}
