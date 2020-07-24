#pragma once

#include <iostream>
#include "types.h"
#include <imagium/Config.h>

namespace imagium {

    class converter {
    public:
        struct result {
            byte_array data;
            uint size_bytes;
        };

        converter() = default;

//        virtual std::vector<ubyte> write(byte_array * $data, const options & options) const = 0;
        virtual result write(byte_array * $data, const Config & options) const = 0;
    };
}
