#pragma once

namespace microgl {
    namespace functions {
        using bits= unsigned char;

        template <typename Integer>
        bits used_integer_bits(const Integer &value) {
            // todo:: make it a binary search instead to get O(log(bits)) instead of linear O(bits)
            const Integer abs_value= value<0 ? -value:value;
            bits bits_used=0;
            while (abs_value>Integer(1)<<(bits_used++)) {};
            return bits_used-1;
        }

    }
}