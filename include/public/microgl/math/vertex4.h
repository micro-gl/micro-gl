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

#ifndef MICROGL_USE_EXTERNAL_MICRO_TESS
#include "../micro-tess/include/micro-tess/vec4.h"
#else
#include <micro-tess/vec4.h>
#endif
#include "vertex3.h"

namespace microgl {
    template<typename number>
    using vertex4 = microtess::vec4<number>;
}
