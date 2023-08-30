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

#include <microgl/math/matrix_4x4.h>
#include <microgl/shaders/shader.h>

namespace microgl {
    namespace shading {

        template<typename number_, typename rgba_>
        struct color_shader {
            // the main number type
            using number = number_;
            // the rgba info
            using rgba = rgba_;
            using gl_position= vertex4<number>;

            // per vertex attributes
            struct vertex_attributes {
                vertex3<number> point;
                microgl::color_t color;
            };

            // varying attributes
            struct varying {
                microgl::color_t color{255,0,0};

                // you must implement the interpolation function
//                template <typename bary_integer>
                void interpolate(const varying &varying_a,
                                 const varying &varying_b,
                                 const varying &varying_c,
                                 const vertex4<int> &bary) {
                    color.r = int(vertex4<number>(varying_a.color.r, varying_b.color.r, varying_c.color.r, 0).dot(vertex4<number>(bary)) / bary.w);
                    color.g = int(vertex4<number>(varying_a.color.g, varying_b.color.g, varying_c.color.g, 0).dot(vertex4<number>(bary)) / bary.w);
                    color.b = int(vertex4<number>(varying_a.color.b, varying_b.color.b, varying_c.color.b, 0).dot(vertex4<number>(bary)) / bary.w);
                    color.a = int(vertex4<number>(varying_a.color.a, varying_b.color.a, varying_c.color.a, 0).dot(vertex4<number>(bary)) / bary.w);
                }
            };

            // this is a uniform global
            matrix_4x4<number> matrix;

            // vertex shader
            inline gl_position
            vertex(const vertex_attributes &attributes, varying &output) {
                // pass varying on to the pixel shader so they can be interpolated
                output.color = attributes.color;

                // tag the transformed position
                return matrix * gl_position{attributes.point};
            }

            // pixel shader
            inline microgl::color_t
            fragment(const varying &input) {
                return input.color;
            }

        };

    }

}
