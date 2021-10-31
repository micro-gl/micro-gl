#pragma once

#include "../crpt.h"
#include "../vec4.h"
#include "../color.h"

namespace microgl {
    namespace shading {

        template<class shader>
        using vertex_attributes = typename shader::vertex_attributes;

        template<class shader>
        using varying = typename shader::varying;

        template<class shader>
        using shader_number = typename shader::number;

        template<class shader>
        using shader_rgba = typename shader::rgba;

        template<typename rgba_, typename impl, typename vertex_attr, typename varying, typename number>
        class shader_base : public crpt<impl> {
        protected:
            using vertex3= vec3<number>;
            using vertex4= vec4<number>;
            using gl_position= vec4<number>;
            using gl_FragCoord= vec4<number>;
            gl_FragCoord _frag_coord;
        public:
            using rgba = rgba_;

            //            const vertex4 &barycentric() {
//                return _bary;
//            }
//
//            const gl_FragCoord & fragCoord() {
//                return _frag_coord;
//            }
            gl_FragCoord fragCoord;
            vertex4 _bary;

            inline gl_position vertex(const vertex_attr &attributes, varying &output) {
                return this->derived().vertex(attributes, output);
            }

            inline color::color_t fragment(const varying &input) {
                return this->derived().fragment(input);
            }

        };

    }

}