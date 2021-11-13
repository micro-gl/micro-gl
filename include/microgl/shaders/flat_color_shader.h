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

#include <microgl/shaders/shader.h>

namespace microgl {
    namespace shading {

        template<typename number_, typename rgba_>
        struct flat_color_shader {
            using number = number_;
            using rgba = rgba_;
            using gl_position= vertex4<number>;

            struct vertex_attributes {
                vertex3<number> point;
            };

            struct varying {
                template <typename bary_integer>
                void interpolate(const varying &varying_a,
                                 const varying &varying_b,
                                 const varying &varying_c,
                                 const vertex4<bary_integer> &bary) {
                }
            };

            color_t color;
            matrix_4x4<number> matrix;

            inline gl_position
            vertex(const vertex_attributes &attributes, varying &output) {
                return matrix * vertex4<number>{attributes.point};
            }

            inline microgl::color_t
            fragment(const varying &input) {
                return color;
            }

        };

    }

}
