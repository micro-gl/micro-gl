#pragma once

namespace microgl {
    namespace numbers {
        int to_fixed(float val, unsigned char precision) {
            return int(val*float(1<<precision));
        }
    }
}