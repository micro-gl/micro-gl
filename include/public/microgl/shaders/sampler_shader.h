#pragma once

#include <microgl/shader.h>
#include <microgl/sampler.h>
#include <microgl/precision.h>

namespace microgl {
    namespace shading {

        template<typename number_, typename S>
        struct sampler_shader {

            using rgba = typename S::rgba;
            using number = number_;


            struct vertex_attributes {
                vec3<number> point;
                vec2<number> uv;
            };

            struct varying {
                vec2<number> uv;

                void interpolate(const varying &varying_a,
                                 const varying &varying_b,
                                 const varying &varying_c,
                                 const vec4<int> &bary) {
                    uv.x = (varying_a.uv.x*bary.x + varying_b.uv.x*bary.y + varying_c.uv.x*bary.z)/bary.w;
                    uv.y = (varying_a.uv.y*bary.x + varying_b.uv.y*bary.y + varying_c.uv.y*bary.z)/bary.w;
                }
            };

            matrix_4x4<number> matrix;
            S *sampler= nullptr;

            inline vec4<number>
            vertex(const vertex_attributes &attributes, varying &output) {
                output.uv = attributes.uv;
                return matrix * vec4<number>{attributes.point};
            }

            inline color::color_t
            fragment(const varying &input) {
                color_t output;
                sampler->sample(input.uv.x, input.uv.y, microgl::precision::high, output);
                return output;
            }

        };

    }

}