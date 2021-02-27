#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/shader.h>
#include <microgl/matrix_4x4.h>

namespace microgl {
    namespace shading {

        template <typename number>
        struct color_shader_varying {
            microgl::color::color_t color{255,0,0};

            void interpolate(const color_shader_varying &varying_a,
                             const color_shader_varying &varying_b,
                             const color_shader_varying &varying_c,
                             const vec4<int> &bary) {
                color.r = int(vec4<number>{varying_a.color.r, varying_b.color.r, varying_c.color.r, 0}.dot(vec4<number>(bary))/bary.w);
                color.g = int(vec4<number>{varying_a.color.g, varying_b.color.g, varying_c.color.g, 0}.dot(vec4<number>(bary))/bary.w);
                color.b = int(vec4<number>{varying_a.color.b, varying_b.color.b, varying_c.color.b, 0}.dot(vec4<number>(bary))/bary.w);
                color.a = int(vec4<number>{varying_a.color.a, varying_b.color.a, varying_c.color.a, 0}.dot(vec4<number>(bary))/bary.w);
            }
        };

        template<typename number>
        struct color_shader_vertex_attributes {
            vec3<number> point;
            microgl::color::color_t color;
        };

        template<typename number, uint8_t r, uint8_t g, uint8_t b, uint8_t a>
        class color_shader : public shader_base<r,g,b,a, color_shader<number, r,g,b,a>, color_shader_vertex_attributes<number>, color_shader_varying<number>, number> {
        public:
            matrix_4x4<number> matrix;

            inline vec4<number>
            vertex(const color_shader_vertex_attributes<number> &attributes, color_shader_varying<number> &output) {
                output.color = attributes.color;
                auto result= matrix * vec4<number>{attributes.point};
                return result;
            }

            inline color::color_t fragment(const color_shader_varying<number> &input) {
                return input.color;
            }

        };

    }

}

#pragma clang diagnostic pop