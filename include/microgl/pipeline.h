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

#include "math/matrix_4x4.h"
#include "math/vertex2.h"
#include "math/vertex3.h"
#include "color.h"
#ifndef MICROGL_USE_EXTERNAL_MICRO_TESS
#include "micro-tess/include/micro-tess/triangles.h"
#else
#include <micro-tess/triangles.h>
#endif

namespace microgl {
    namespace _3d {
        /**
         * this is a simple fixed 3d pipeline interface
         */
        template <typename number, class canvas_type>
        class pipeline {
        public:
            using index = unsigned;
            using const_ref = const number &;
            using vertex2 = microgl::vertex2<number>;
            using vertex3 = microgl::vertex3<number>;
            using vertex4 = microgl::vertex4<number>;
            using mat4 = matrix_4x4<number>;

            static
            vertex3 world_to_raster_space(const vertex3 &world, const mat4 &mvp, index width, index height) {
                // given world coord, transform it to raster/canvas space
                vertex4 clip_space= mvp*vertex4{world};
                // now perform clipping on the [-w,w]x[-w,w]x[-w,w] && w>0 cube
                // todo:: implement clipping, although it might be problematic
                //     :: I need to infer bary multipliers so user can clip attributes
                //     :: also, maybe only go for culling and clipping on the z-plane
                // now convert to NDC space:: [-1,1]x[-1,1]x[-1,1]
                clip_space = clip_space/clip_space.w;
                // now convert to raster space
                return viewport(clip_space, width, height);
            }

            static vertex3 viewport(const vertex4 &ndc, index width, index height) {
                // given NDC= Normalized Device Coordinates, then transform them into
                // raster/canvas/viewport coords. We assume, that NDC coords are [-1,1] range.
                // todo:; currently I assume no z clipping has occured
                // z value is mapped to [0,1] range
                // convert to raster space
                const_ref zero=number(0), one = number(1), two=number(2);
                vertex3 result{};
                result.x = ((ndc.x + one)*width)/two;
                result.y = number(height) - (((ndc.y + one)*number(height))/two);
                result.z = (ndc.z + one)/two;
                // z clamping
                if(result.z<zero)
                    result.z = zero;
                if(result.z>one)
                    result.z = one;

                return result;
            }

            static void render(const color_t & color,
                        const vertex3 * vertices,
                        const index vertices_size,
                        const index * indices,
                        const index indices_size,
                        const mat4 & mvp,
                        const microtess::triangles::indices & type,
                        canvas_type & canva,
                        const microgl::ints::uint8_t opacity=255) {
                // todo:: noe this renders lines but does not clip them to the frustum, but only to the viewport
                // this will cause strange effects due to the twisting mirror effect of the projection matrix
#define IND(a) indices[(a)]
                const unsigned width = canva.width();
                const unsigned height = canva.height();

                switch (type) {
                    case microtess::triangles::indices::TRIANGLES:
                    case microtess::triangles::indices::TRIANGLES_WITH_BOUNDARY:

                        for (index ix = 0; ix < indices_size; ix+=3) {

                            vertex3 v1 = world_to_raster_space(vertices[IND(ix + 0)], mvp, width, height);
                            vertex3 v2 = world_to_raster_space(vertices[IND(ix + 1)], mvp, width, height);
                            vertex3 v3 = world_to_raster_space(vertices[IND(ix + 2)], mvp, width, height);

                            vertex2 v1_ = {v1.x, v1.y};
                            vertex2 v2_ = {v2.x, v2.y};
                            vertex2 v3_ = {v3.x, v3.y};

                            canva.drawTriangleWireframe(color, v1_, v2_, v3_, opacity);
                        }

                        break;
                    default: break;
//                    case indices::TRIANGLES_FAN:
//                    case indices::TRIANGLES_FAN_WITH_BOUNDARY:
//
//                        for (index ix = 1; ix < indices_size-1; ++ix) {
//                            vertex3 v1 = camera<number>::world_to_raster_space(vertices[IND( 0)], mvp, width, height);
//                            vertex3 v2 = camera<number>::world_to_raster_space(vertices[IND(ix)], mvp, width, height);
//                            vertex3 v3 = camera<number>::world_to_raster_space(vertices[IND(ix + 1)], mvp, width, height);
//
//                            canva.drawTriangleWireframe(color, v1, v2, v3);
//                        }
//
//                        break;
//
//                    case indices::TRIANGLES_STRIP:
//                    case indices::TRIANGLES_STRIP_WITH_BOUNDARY:
//                    {
//                        bool even = true;
//
//                        for (index ix = 0; ix < indices_size-2; ++ix) {
//                            // we alternate order inorder to preserve CCW or CW,
//                            // in the future I will add face culling, which will
//                            // support only CW or CCW orientation_t at a time.
//                            vertex3 v1, v2, v3;
//
//                            if(even) {
//                                v1 = camera<number>::world_to_raster_space(vertices[IND(ix + 0)], mvp, width, height);
//                                v2 = camera<number>::world_to_raster_space(vertices[IND(ix + 1)], mvp, width, height);
//                                v3 = camera<number>::world_to_raster_space(vertices[IND(ix + 2)], mvp, width, height);
//                            } else {
//                                v1 = camera<number>::world_to_raster_space(vertices[IND(ix + 2)], mvp, width, height);
//                                v2 = camera<number>::world_to_raster_space(vertices[IND(ix + 1)], mvp, width, height);
//                                v3 = camera<number>::world_to_raster_space(vertices[IND(ix + 0)], mvp, width, height);
//                            }
//
//                            canva.drawTriangleWireframe(color, v1, v2, v3);
//                            even = !even;
//                        }
//
//                        break;
//                    }
                }
#undef IND
            }
        };
    }
}