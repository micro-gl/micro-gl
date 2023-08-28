/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "traits.h"

namespace microgl {

    namespace ints {

        using int8_t = signed char;
        using uint8_t = unsigned char;

        using int16_t = signed short;
        using uint16_t = unsigned short;

        using int32_t = signed int;
        using uint32_t = unsigned int;

        using int64_t = signed long long;
        using uint64_t = unsigned long long;

        template<microgl::ints::uint8_t BITS>
        struct unsigned_type_infer {
            static constexpr microgl::ints::uint8_t bits = BITS;
            using type = typename microgl::traits::conditional<
                    BITS <= 8, microgl::ints::uint8_t,
                    typename microgl::traits::conditional<
                            BITS <= 16, microgl::ints::uint16_t,
                            typename microgl::traits::conditional<
                                    BITS <= 32, microgl::ints::uint32_t, microgl::ints::uint64_t>::type>::type>::type;
        };

        template<microgl::ints::uint8_t BITS>
        struct signed_type_infer {
            static constexpr microgl::ints::uint8_t bits = BITS;
            using type = typename microgl::traits::conditional<
                    BITS <= 8, microgl::ints::int8_t,
                    typename microgl::traits::conditional<
                            BITS <= 16, microgl::ints::int16_t,
                            typename microgl::traits::conditional<
                                    BITS <= 32, microgl::ints::int32_t, microgl::ints::int64_t>::type>::type>::type;
        };

        template<microgl::ints::uint8_t bits>
        using uint_t = typename unsigned_type_infer<bits>::type;

        template<microgl::ints::uint8_t bits>
        using int_t = typename signed_type_infer<bits>::type;
    }
}