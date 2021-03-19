#pragma once

#include <microgl/micro_gl_traits.h>

using int8_t = signed char;
using uint8_t = unsigned char;

using int16_t = signed short;
using uint16_t = unsigned short;

using int32_t = signed int;
using uint32_t = unsigned int;

using int64_t = signed long long;
using uint64_t = unsigned long long;

template<uint8_t BITS>
struct unsigned_type_infer {
    static constexpr uint8_t bits = BITS;
    using type = typename microgl::traits::conditional<
            BITS<=8, uint8_t ,
            typename microgl::traits::conditional<
                    BITS<=16, uint16_t ,
                    typename microgl::traits::conditional<
                            BITS<=32, uint32_t , uint64_t >::type>::type>::type;
};

template<uint8_t BITS>
struct signed_type_infer {
    static constexpr uint8_t bits = BITS;
    using type = typename microgl::traits::conditional<
            BITS<=8, int8_t ,
            typename microgl::traits::conditional<
                    BITS<=16, int16_t ,
                    typename microgl::traits::conditional<
                            BITS<=32, int32_t , int64_t >::type>::type>::type;
};

template<uint8_t bits>
using uint_t = typename unsigned_type_infer<bits>::type;

template<uint8_t bits>
using int_t = typename signed_type_infer<bits>::type;
