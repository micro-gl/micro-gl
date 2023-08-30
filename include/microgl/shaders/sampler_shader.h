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
#include <microgl/samplers/sampler.h>
#include <microgl/samplers/precision.h>

namespace microgl {
    namespace shading {

        template<typename number_, class S>
        struct sampler_shader {

            using rgba = typename S::rgba;
            using number = number_;

            struct vertex_attributes {
                vertex3<number> point;
                vertex2<number> uv;
            };

            struct varying {
                vertex2<number> uv;

                void interpolate(const varying &varying_a,
                                 const varying &varying_b,
                                 const varying &varying_c,
                                 const vertex4<int> &bary) {
                    uv.x = (varying_a.uv.x*bary.x + varying_b.uv.x*bary.y + varying_c.uv.x*bary.z)/bary.w;
                    uv.y = (varying_a.uv.y*bary.x + varying_b.uv.y*bary.y + varying_c.uv.y*bary.z)/bary.w;
                }
            };

            matrix_4x4<number> matrix;
            S *sampler= nullptr;

            inline vertex4<number>
            vertex(const vertex_attributes &attributes, varying &output) {
                output.uv = attributes.uv;
                return matrix * vertex4<number>{attributes.point};
            }

            inline microgl::color_t
            fragment(const varying &input) {
                color_t output;
                sampling::sample(*sampler, input.uv.x, input.uv.y,
                                 microgl::precision::high, output);
                return output;
            }

        };

    }

}