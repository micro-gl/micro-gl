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

            inline color::color_t
            fragment(const varying &input) {
                return color;
            }

        };

    }

}
