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

#include "microgl/math/vertex2.h"
#include "microgl/math/vertex3.h"
#include "microgl/math/vertex4.h"
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
        class shader_base : public microgl::traits::crpt<impl> {
        protected:
            using vertex3= microgl::vertex3<number>;
            using vertex4= microgl::vertex4<number>;
            using gl_position= vertex4;
            using gl_FragCoord= vertex4;
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

            inline microgl::color_t fragment(const varying &input) {
                return this->derived().fragment(input);
            }

        };

    }

}