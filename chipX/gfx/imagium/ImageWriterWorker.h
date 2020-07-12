#pragma once

#include <iostream>
#include <types.h>

namespace imagium {

    class ImageWriterWorker {
    public:
        ImageWriterWorker() = default;

        virtual std::vector<ubyte> write(byte_array * $data, const options & options) const = 0;
    };
}
