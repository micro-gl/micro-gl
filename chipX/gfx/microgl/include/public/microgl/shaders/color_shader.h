#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/shader.h>

namespace microgl {
    namespace shading {

        template<typename number>
        struct color_shader_varying {
            microgl::color::color_t color{255,0,0};

            void interpolate(const color_shader_varying &varying_a,
                             const color_shader_varying &varying_b,
                             const color_shader_varying &varying_c,
                             const vec4<long long> &bary) {
                color.r = number((vec4<long long>{varying_a.color.r, varying_b.color.r, varying_c.color.r, 0} * bary)/bary.w);
                color.g = number((vec4<long long>{varying_a.color.g, varying_b.color.g, varying_c.color.g, 0} * bary)/bary.w);
                color.b = number((vec4<long long>{varying_a.color.b, varying_b.color.b, varying_c.color.b, 0} * bary)/bary.w);
                color.a = number((vec4<long long>{varying_a.color.a, varying_b.color.a, varying_c.color.a, 0} * bary)/bary.w);
            }
        };

        template<typename number>
        struct color_shader_vertex_attributes {
            vec3<number> point;
            microgl::color::color_t color;
        };

        template<typename number>
        class color_shader : public shader_base<color_shader<number>, color_shader_vertex_attributes<number>, color_shader_varying<number>, number> {
        public:
            matrix_4x4<number> mat;

            inline vec4<number>
            vertex(const color_shader_vertex_attributes<number> &attributes, color_shader_varying<number> &output) {
                output.color = attributes.color;
                auto result= mat * vec4<number>{attributes.point};
                return result;
            }

            inline color::color_t fragment(const color_shader_varying<number> &input) {
                return input.color;
            }

        };

    }

}

#pragma clang diagnostic pop