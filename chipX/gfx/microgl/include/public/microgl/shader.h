#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma once

#include <microgl/crpt.h>
#include <microgl/vec4.h>
#include <microgl/color.h>
#include <microgl/camera.h>

namespace microgl {
    namespace shading {

        template<typename number>
        struct simple_varying {
            color_t color{255,0,0};

            void interpolate(const simple_varying &varying_a,
                             const simple_varying &varying_b,
                             const simple_varying &varying_c,
                             const vec4<long long> &bary) {
                color.r = number((vec4<long long>{varying_a.color.r, varying_b.color.r, varying_c.color.r, 0} * bary)/bary.w);
                color.g = number((vec4<long long>{varying_a.color.g, varying_b.color.g, varying_c.color.g, 0} * bary)/bary.w);
                color.b = number((vec4<long long>{varying_a.color.b, varying_b.color.b, varying_c.color.b, 0} * bary)/bary.w);
                color.a = number((vec4<long long>{varying_a.color.a, varying_b.color.a, varying_c.color.a, 0} * bary)/bary.w);
            }
        };

        template<typename number>
        struct simple_vertex_attributes {
            vec3<number> point;
            color_t color;
        };

        template<typename impl, typename vertex_attr, typename varying, typename number>
        class shader_base : public crpt<impl> {
        protected:
            using vertex3= vec3<number>;
            using vertex4= vec4<number>;
            using gl_position= vec4<number>;
            vertex4 _bary;
        public:
            const vertex4 &barycentric() {
                return _bary;
            }

            inline gl_position vertex(const vertex_attr &attributes, varying &output) {
                return this->derived().vertex(attributes, output);
            }

            inline color::color_t fragment(const varying &input) {
                return this->derived().fragment(input);
            }

        };

        class color_shader : public shader_base<color_shader, simple_vertex_attributes<float>, simple_varying<float>, float> {
        public:
            matrix_4x4<float> mat = camera<float>::orthographic(0, 640, 0, 640, 1, 100);

            inline gl_position
            vertex(const simple_vertex_attributes<float> &attributes, simple_varying<float> &output) {
                output.color = attributes.color;
                auto result= mat * vec4<float>{attributes.point};
                return result;
            }

            inline color::color_t fragment(const simple_varying<float> &input) {
                return input.color;
            }

        };

    }

}
#pragma clang diagnostic pop