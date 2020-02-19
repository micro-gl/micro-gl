#pragma once

namespace microgl {
    using precision_t = unsigned char;

    enum class precision {
        none=0, low=5, medium=10, high=15, amazing=24
    };
}