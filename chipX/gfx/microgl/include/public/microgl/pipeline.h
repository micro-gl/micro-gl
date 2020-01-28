#pragma once

#include <microgl/matrix_4x4.h>
#include <microgl/vec2.h>
#include <microgl/vec3.h>
#include <microgl/triangles.h>
#include <microgl/color.h>

namespace microgl {
    namespace _3d {

        template <typename number, class canvas_type>
        class pipeline {
            /**
             * this is a simple fixed 3d pipeline interface
             */
        private:
            using index = unsigned;
            using const_ref = const number &;
            using vertex2 = vec2<number>;
            using vertex3 = vec3<number>;
            using vertex4 = vec4<number>;
            using mat4 = matrix_4x4<number>;
        public:

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

            static
            vertex3 viewport(const vertex4 &ndc, index width, index height) {
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

            //            template <>
            static
            void render(const vertex3 * vertices,
                        const index vertices_size,
                        const index * indices,
                        const index indices_size,
                        const mat4 & mvp,
                        const triangles::indices & type,
                        canvas_type & canva) {

#define IND(a) indices[(a)]
                color::color_f_t color = color::colors::BLACK;
                const unsigned width = canva.width();
                const unsigned height = canva.height();

                switch (type) {
                    case triangles::indices::TRIANGLES:
                    case triangles::indices::TRIANGLES_WITH_BOUNDARY:

                        for (index ix = 0; ix < indices_size; ix+=3) {

                            vertex3 v1 = world_to_raster_space(vertices[IND(ix + 0)], mvp, width, height);
                            vertex3 v2 = world_to_raster_space(vertices[IND(ix + 1)], mvp, width, height);
                            vertex3 v3 = world_to_raster_space(vertices[IND(ix + 2)], mvp, width, height);

                            vertex2 v1_ = {v1.x, v1.y};
                            vertex2 v2_ = {v2.x, v2.y};
                            vertex2 v3_ = {v3.x, v3.y};

                            canva.drawTriangleWireframe(color, v1_, v2_, v3_);
                        }

                        break;
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