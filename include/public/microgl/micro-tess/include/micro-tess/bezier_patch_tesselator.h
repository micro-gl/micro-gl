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

#include "triangles.h"
#include "vec3.h"

namespace microtess {

    template <typename number1, typename number2, class container_output_attributes,
            class container_output_indices>
    class bezier_patch_tesselator {
        using index = unsigned int;
        using vertex3 = microtess::vec3<number1>;
    public:

        bezier_patch_tesselator()=delete;
        bezier_patch_tesselator(const bezier_patch_tesselator &)=delete;
        bezier_patch_tesselator(bezier_patch_tesselator &&)=delete;
        bezier_patch_tesselator & operator=(const bezier_patch_tesselator &)=delete;
        bezier_patch_tesselator & operator=(bezier_patch_tesselator &&)=delete;
        ~bezier_patch_tesselator()=delete;

        static const char BI_QUADRATIC = 0;
        static const char BI_CUBIC = 1;

        /**
         * right now I only support cubic and quadratic patches.
         * todo: support arbitrary patches
         *
         */
        static void compute(const vertex3 *meshPoints,
                     const index uOrder, const index vOrder,
                     const index uSamples, const index vSamples,
                     container_output_attributes &out_vertices_attributes,
                     container_output_indices &out_indices,
                     triangles::indices & out_indices_type,
                     number2 tex_left=number2(0), number2 tex_top=number2(1),
                     number2 tex_right=number2(1), number2 tex_bottom=number2(0)) {
            bool supported= uOrder == vOrder && (uOrder == 3 || uOrder == 4);
            if(!supported) return;
            const char type= uOrder == 3 ? BI_QUADRATIC : BI_CUBIC;

            auto du = number2(1) / number2(uSamples - 1);
            auto dv = number2(1) / number2(vSamples - 1);

            number2 factor_remap_u = (tex_right - tex_left);
            number2 factor_remap_v = (tex_bottom-tex_top);

            // iterates top to bottom and left to right
            // use the parametric time value 0 to 1
            for(index i=0; i < uSamples; i++) { // left to right
                number2 u = number2(i) * du;
                for(index j=0; j < vSamples; j++) { // top to bottom
                    // calculate the parametric v value
                    number2 v = number2(j) * dv;
                    // calculate the point on the surface
                    vertex3 p;
                    if(type==BI_QUADRATIC)
                        p = evaluateBiQuadraticSurface(u,v, meshPoints);
                    else if (type==BI_CUBIC)
                        p = evaluateBiCubicSurface(u,v, meshPoints);
                    else return;

                    //int idx = i * vSamples + j;
                    out_vertices_attributes.push_back(p.x);
                    out_vertices_attributes.push_back(p.y);
                    out_vertices_attributes.push_back(p.z);
                    // we need to separate this into another buffer, because number1!=number2 always
                    out_vertices_attributes.push_back(tex_left + u*factor_remap_u);
                    out_vertices_attributes.push_back(tex_top + v*factor_remap_v);

                    // indices
                    if(i < uSamples - 1) {
                        out_indices.push_back(i*vSamples + j);
                        out_indices.push_back((i+1)*vSamples + j);

                        // degenerate cases
                        if((j==vSamples-1) && (i < uSamples - 2)){
                            out_indices.push_back((i+1)*vSamples + j);
                            out_indices.push_back((i+1)*vSamples);
                            // i think it is missing another one
                        }
                    }
                }
            }
        }

        static
        vertex3 evaluateBiCubicSurface(number1 u, number1 v, const vertex3 *meshPoints
//                                           vertex3 (*meshPoints)[4],
                                       ) {
            vertex3 temp[4], result;

            // 3rd degree bernstein polynomials coefficients
            // the t value inverted
            number1 t = u;
            number1 it = number1(1) - t;
            number1 b0 = it * it * it;
            number1 b1 = number1(3) * t * it * it;
            number1 b2 = number1(3) * t * t * it;
            number1 b3 = t * t * t;

            // first of all we will need to evaluate 4 curves in the u
            // direction. The points from_sampler those will be stored in this
            // temporary array
            for (index ix = 0; ix < 4; ++ix) {
                index idx=ix*4;
                temp[ix].x = b0*meshPoints[idx+0].x + b1*meshPoints[idx+1].x + b2*meshPoints[idx+2].x + b3*meshPoints[idx+3].x;
                temp[ix].y = b0*meshPoints[idx+0].y + b1*meshPoints[idx+1].y + b2*meshPoints[idx+2].y + b3*meshPoints[idx+3].y;
                temp[ix].z = b0*meshPoints[idx+0].z + b1*meshPoints[idx+1].z + b2*meshPoints[idx+2].z + b3*meshPoints[idx+3].z;
            }

            t = v;
            it = number1(1) - t;
            b0 = it*it*it;
            b1 = number1(3) * t * it * it;
            b2 = number1(3) * t * t * it;
            b3 =  t*t*t;

            // having got 4 points, we can use it as a bezier curve
            // to calculate the v direction. This should give us our
            // final point
            result.x = b0*temp[0].x + b1*temp[1].x + b2*temp[2].x + b3*temp[3].x;
            result.y = b0*temp[0].y + b1*temp[1].y + b2*temp[2].y + b3*temp[3].y;
            result.z = b0*temp[0].z + b1*temp[1].z + b2*temp[2].z + b3*temp[3].z;

            return result;
        };

        static
        vertex3 evaluateBiQuadraticSurface(number1 u, number1 v, const vertex3 *meshPoints) {
            vertex3 temp[3], result;

            // 2rd degree bernstein polynomials coefficients
            // the t value inverted
            number1 t = u;
            number1 it = number1(1) - t;
            number1 b0 = it * it;
            number1 b1 = number1(2) * t * it;
            number1 b2 = t * t;

            // first of all we will need to evaluate 4 curves in the u
            // direction. The points from_sampler those will be stored in this
            // temporary array
            for (index ix = 0; ix < 3; ++ix) {
                index idx=ix*3;
                temp[ix].x = b0*meshPoints[idx+0].x + b1*meshPoints[idx+1].x + b2*meshPoints[idx+2].x;
                temp[ix].y = b0*meshPoints[idx+0].y + b1*meshPoints[idx+1].y + b2*meshPoints[idx+2].y;
                temp[ix].z = b0*meshPoints[idx+0].z + b1*meshPoints[idx+1].z + b2*meshPoints[idx+2].z;
            }

            t = v;
            it = number1(1) - t;
            b0 = it*it;
            b1 = number1(2) * t * it;
            b2 = t*t;

            // having got 3 points, we can use it as a bezier curve
            // to calculate the v direction. This should give us our
            // final point
            result.x = b0*temp[0].x + b1*temp[1].x + b2*temp[2].x;
            result.y = b0*temp[0].y + b1*temp[1].y + b2*temp[2].y;
            result.z = b0*temp[0].z + b1*temp[1].z + b2*temp[2].z;

            return result;
        };
    };
}
