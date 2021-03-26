#pragma once

namespace microgl {
    namespace math {

        template<typename number>
        static
        number abs(const number &val) {
            return val < 0 ? -val : val;
        }

    };
}