#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/shader.h>
#include <microgl/Bitmap.h>
#include <microgl/precision.h>

namespace microgl {
    namespace shading {

        template<typename number>
        struct texture_shader_varying {
            vec2<number> uv;

            void interpolate(const texture_shader_varying &varying_a,
                             const texture_shader_varying &varying_b,
                             const texture_shader_varying &varying_c,
                             const vec4<long long> &bary) {
                uv.x = (varying_a.uv.x*bary.x + varying_b.uv.x*bary.y + varying_c.uv.x*bary.z)/bary.w;
                uv.y = (varying_a.uv.y*bary.x + varying_b.uv.y*bary.y + varying_c.uv.y*bary.z)/bary.w;
//                uv.x = (vec4<number>{varying_a.uv.x, varying_b.uv.x, varying_c.uv.x, 0} * vec4<number>{bary})/number(bary.w);
//                uv.y = (vec4<number>{varying_a.uv.y, varying_b.uv.y, varying_c.uv.y, 0} * vec4<number>{bary})/number(bary.w);
            }
        };

        template<typename number>
        struct texture_shader_vertex_attributes {
            vec3<number> point;
            vec2<number> uv;
        };

        template<typename number, typename P, typename CODER, typename Sampler>
        class texture_shader : public shader_base<
                texture_shader<number, P, CODER, Sampler>,
                texture_shader_vertex_attributes<number>,
                texture_shader_varying<number>, number> {
        public:
            matrix_4x4<number> matrix= camera<number>::orthographic(0, 640, 0, 640, 1, 100);
            Bitmap<P, CODER> *texture= nullptr;

            inline vec4<number>
            vertex(const texture_shader_vertex_attributes<number> &attributes, texture_shader_varying<number> &output) {
                output.uv = attributes.uv;
                return matrix * vec4<number>{attributes.point};
            }

            inline color::color_t
            fragment(const texture_shader_varying<number> &input) {
                color_t output;
                Sampler::sampleUnit(*texture, input.uv.x, input.uv.y, microgl::precision::low, output);
                return output;
//                return {255,0,0};
            }

        };

    }

}

#pragma clang diagnostic pop