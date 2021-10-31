#pragma once

#include "math/base_math.h"
#include "math/int_math.h"

#ifdef MICROGL_AVOID_BUILTIN_MATH
#elif defined(MICROGL_USE_STD_MATH)
#include <microgl/math/std_q_math.h>
#include <microgl/math/std_float_math.h>
#else
#include <microgl/math/non_std_q_math.h>
#include <microgl/math/non_std_float_math.h>
#endif

namespace microgl {
    namespace math {
    };
}