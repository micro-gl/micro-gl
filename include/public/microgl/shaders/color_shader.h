#pragma once

#include <microgl/shader.h>
#include <microgl/matrix_4x4.h>

namespace microgl {
    namespace shading {

        template<typename number_, typename rgba_>
        struct color_shader {
            // the main number type
            using number = number_;
            // the rgba info
            using rgba = rgba_;
            using gl_position= vec4<number>;

            // per vertex attributes
            struct vertex_attributes {
                vec3<number> point;
                microgl::color::color_t color;
            };

            // varying attributes
            struct varying {
                microgl::color::color_t color{255,0,0};

                // you must implement the interpolation function
                void interpolate(const varying &varying_a,
                                 const varying &varying_b,
                                 const varying &varying_c,
                                 const vec4<int> &bary) {
                    color.r = int(vec4<number>{varying_a.color.r, varying_b.color.r, varying_c.color.r, 0}.dot(vec4<number>(bary))/bary.w);
                    color.g = int(vec4<number>{varying_a.color.g, varying_b.color.g, varying_c.color.g, 0}.dot(vec4<number>(bary))/bary.w);
                    color.b = int(vec4<number>{varying_a.color.b, varying_b.color.b, varying_c.color.b, 0}.dot(vec4<number>(bary))/bary.w);
                    color.a = int(vec4<number>{varying_a.color.a, varying_b.color.a, varying_c.color.a, 0}.dot(vec4<number>(bary))/bary.w);
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
            inline color::color_t
            fragment(const varying &input) {
                return input.color;
            }

        };

    }

}
