#pragma once

#include <microgl/camera.h>
#include <microgl/matrix_4x4.h>
#include <microgl/math.h>
#include <microgl/vec2.h>
#include <microgl/vec3.h>
#include <microgl/triangles.h>
#include <microgl/color.h>
#include <microgl/canvas.h>


namespace microgl {
    namespace _3d {

        template <typename number, class canvas>
        class pipeline {
            /**
             * this is a simple 3d pipeline sugar interface
             */
        private:
            using index = unsigned;
            using const_ref = const number &;
            using vertex2 = vec2<number>;
            using vertex3 = vec3<number>;
            using mat4 = matrix_4x4<number>;
        public:

//            template <>
            static
            void render(const vertex3 * vertices,
                        const index vertices_size,
                        const index * indices,
                        const index indices_size,
                        const mat4 & mvp,
                        const triangles::indices & type,
                        canvas & canva) {

#define IND(a) indices[(a)]
                color_f_t color = BLACK;
                const unsigned width = canva.width();
                const unsigned height = canva.height();

                switch (type) {
                    case indices::TRIANGLES:
                    case indices::TRIANGLES_WITH_BOUNDARY:

                        for (index ix = 0; ix < indices_size; ix+=3) {

                            vertex3 v1 = camera<number>::world_to_raster_space(vertices[IND(ix + 0)], mvp, width, height);
                            vertex3 v2 = camera<number>::world_to_raster_space(vertices[IND(ix + 1)], mvp, width, height);
                            vertex3 v3 = camera<number>::world_to_raster_space(vertices[IND(ix + 2)], mvp, width, height);

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