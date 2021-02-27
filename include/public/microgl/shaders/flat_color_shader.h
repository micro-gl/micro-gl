#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/shader.h>

namespace microgl {
    namespace shading {

        struct flat_color_shader_varying {
            void interpolate(const flat_color_shader_varying &varying_a,
                             const flat_color_shader_varying &varying_b,
                             const flat_color_shader_varying &varying_c,
                             const vec4<int> &bary) {
            }
        };

        template<typename number>
        struct flat_color_shader_vertex_attributes {
            vec3<number> point;
        };

        template<typename number, uint8_t r, uint8_t g, uint8_t b, uint8_t a>
        class flat_color_shader : public shader_base<r,g,b,a,flat_color_shader<number,r,g,b,a>,
                flat_color_shader_vertex_attributes<number>, flat_color_shader_varying, number> {
        public:
            color_t color;
            matrix_4x4<number> matrix;

            inline vec4<number>
            vertex(const flat_color_shader_vertex_attributes<number> &attributes, flat_color_shader_varying &output) {
                return matrix * vec4<number>{attributes.point};
            }

            inline color::color_t
            fragment(const flat_color_shader_varying &input) {
                return color;
            }

        };

    }

}

#pragma clang diagnostic pop