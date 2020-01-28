#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/shader.h>
#include <microgl/Bitmap.h>

namespace microgl {
    namespace shading {

        template<typename number>
        struct texture_shader_varying {
            vec2<number> uv;

            void interpolate(const texture_shader_varying &varying_a,
                             const texture_shader_varying &varying_b,
                             const texture_shader_varying &varying_c,
                             const vec4<long long> &bary) {
                uv.x = number((vec4<long long>{varying_a.uv.x, varying_b.uv.x, varying_c.uv.x, 0} * bary)/bary.w);
                uv.y = number((vec4<long long>{varying_a.uv.y, varying_b.uv.y, varying_c.uv.y, 0} * bary)/bary.w);
            }
        };

        template<typename number>
        struct texture_shader_vertex_attributes {
            vec3<number> point;
            vec2<number> uv;
        };

        template<typename number, typename P, typename CODER, typename Sampler>
        class texture_shader : public shader_base<texture_shader<number, P, CODER, Sampler>, texture_shader_vertex_attributes<number>,
                texture_shader_varying<number>, number> {
        public:
            matrix_4x4<number> mat = camera<number>::orthographic(0, 640, 0, 640, 1, 100);
            Bitmap<P, CODER> &bmp;

            inline vec4<number>
            vertex(const texture_shader_vertex_attributes<number> &attributes, texture_shader_varying<number> &output) {
                output.color = attributes.color;
                auto result= mat * vec4<number>{attributes.point};
                return result;
            }

            inline color::color_t
            fragment(const texture_shader_varying<number> &input) {
                color_t output;
//                Sampler::sample(bmp, input.uv.x, input.uv.y, BITS_UV_COORDS, output);
                return output;
//                return input.color;
            }

        };

    }

}

#pragma clang diagnostic pop