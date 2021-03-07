#pragma once

#include <microgl/shader.h>

namespace microgl {
    namespace shading {

        template<typename number_, typename rgba_>
        struct flat_color_shader {
            using number = number_;
            using rgba = rgba_;

            struct vertex_attributes {
                vec3<number> point;
            };

            struct varying {
                void interpolate(const varying &varying_a,
                                 const varying &varying_b,
                                 const varying &varying_c,
                                 const vec4<int> &bary) {
                }
            };

            color_t color;
            matrix_4x4<number> matrix;

            inline vec4<number>
            vertex(const vertex_attributes &attributes, varying &output) {
                return matrix * vec4<number>{attributes.point};
            }

            inline color::color_t
            fragment(const varying &input) {
                return color;
            }

        };

    }

}
