#pragma once
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

#include <microgl/shader.h>
#include <microgl/sampler.h>
#include <microgl/precision.h>

namespace microgl {
    namespace shading {

        template<typename number>
        struct sampler_shader_varying {
            vec2<number> uv;

            void interpolate(const sampler_shader_varying &varying_a,
                             const sampler_shader_varying &varying_b,
                             const sampler_shader_varying &varying_c,
                             const vec4<int> &bary) {
//                uv.x = (varying_a.uv.x*bary.x)/bary.w + (varying_b.uv.x*bary.y)/bary.w + (varying_c.uv.x*bary.z)/bary.w;
//                uv.y = (varying_a.uv.y*bary.x)/bary.w + (varying_b.uv.y*bary.y)/bary.w + (varying_c.uv.y*bary.z)/bary.w;

                uv.x = (varying_a.uv.x*bary.x + varying_b.uv.x*bary.y + varying_c.uv.x*bary.z)/bary.w;
                uv.y = (varying_a.uv.y*bary.x + varying_b.uv.y*bary.y + varying_c.uv.y*bary.z)/bary.w;

//                uv.x = vec4<number>{varying_a.uv.x, varying_b.uv.x, varying_c.uv.x, 0}.dot(vec4<number>{bary})/number(bary.w);
//                uv.y = vec4<number>{varying_a.uv.y, varying_b.uv.y, varying_c.uv.y, 0}.dot(vec4<number>{bary})/number(bary.w);
            }
        };

        template<typename number>
        struct sampler_shader_vertex_attribute {
            vec3<number> point;
            vec2<number> uv;
        };

        template<typename number, typename S>
        class sampler_shader : public shader_base<
                sampler_shader<number, S>,
                sampler_shader_vertex_attribute<number>,
                sampler_shader_varying<number>, number> {
        public:
            matrix_4x4<number> matrix;
            sampling::sampler<S> *sampler= nullptr;

            inline vec4<number>
            vertex(const sampler_shader_vertex_attribute<number> &attributes, sampler_shader_varying<number> &output) {
                output.uv = attributes.uv;
                return matrix * vec4<number>{attributes.point};
            }

            inline color::color_t
            fragment(const sampler_shader_varying<number> &input) {
                color_t output;
                sampler->sample(input.uv.x, input.uv.y, microgl::precision::high, output);
//                return {255,255,0};
                return output;
//                return {(unsigned char)(input.uv.x),(unsigned char)(input.uv.y),0};
            }

        };

    }

}

#pragma clang diagnostic pop