#pragma once

#include <types.h>
#include <color_t.h>

namespace imagium {

    class addressable_ram {
    public:
//        virtual void write(unsigned index, uint64_t value) = 0;
        virtual void write(unsigned index, const color_t &color) = 0;
        virtual void write(unsigned index, uint64_t value) = 0;
        virtual color_t operator[](int index) const = 0;
        virtual str toString(const str &name) const = 0;
    };

}
