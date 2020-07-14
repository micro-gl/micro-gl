#pragma once

#include <iostream>
#include <types.h>

namespace imagium {

    class converter {
    public:
        converter() = default;

        virtual std::vector<ubyte> write(byte_array * $data, const options & options) const = 0;
    };
}
