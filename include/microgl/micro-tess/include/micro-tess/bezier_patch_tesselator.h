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

namespace microtess {

    enum class patch_type { BI_QUADRATIC, BI_CUBIC };
    /**
     * BiQuad/BiCubic Bezier Patch Tessellation. Triangulates any dimensional (1d/2d/3d...) surfaces, but usually
     * used for 2d/3d
     *
     * NOTES:
     * 1. Output attributes are stored in interleaved format in the supplied number container, example
     *    output = [x,y,z,u,v, x,y,z,u,v, x,y,z,u,v, ....] (with 3 channels points and uvs output), or
     *    output = [x,y, x,y, x,y,.... ] (with 2 channels points and ignored uvs output)
     * 2. You can decide how many channels a point is 1, 2, 3, 4 etc...
     * 3. You can decide if to output points, uvs or both
     * 4. Output indices of triangulation is stored in a separate indices container
     * 5. the type of triangulation is always TRIANGLES_STRIP to save memory
     * 6. Configurable horizontal/vertical Triangulation resolution
     *
     * @tparam number1 x,y,z number type
     * @tparam number2 u, v number type
     * @tparam container_output_attributes container type of output vertices [x,y,z,u,v,...[
     * @tparam container_output_indices container for indices
     */
    template <typename number1, typename number2,
            class container_output_attributes,
                    class container_output_indices>
    class bezier_patch_tesselator {
        using index = unsigned int;
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
         *
         * @tparam type Patch type { BI_QUADRATIC (9 points), BI_CUBIC (16 points) }
         * @param meshPoints array of (9 or 16) * (channels) for quadratic/cubic respectively
         * @param channels how many components in each point (x, y)=>2, (x,y,z)=>3 etc...
         * @param uSamples number of horizontal subdivisions
         * @param vSamples number of vertical subdivisions
         * @param flag_output_points include interpolated points in output ?
         * @param flag_output_uvs include interpolated uvs in output ?
         * @param out_vertices_attributes output container of attributes [x,y,z,u,v,...]
         * @param out_indices output container for indices
         * @param out_indices_type output triangles type
         * @param tex_left left UV bounding box
         * @param tex_top top UV bounding box
         * @param tex_right right UV bounding box
         * @param tex_bottom bottom UV bounding box
         */
        template<patch_type type>
        static int compute(const number1 *meshPoints,
                     index channels,
                     const index uSamples, const index vSamples,
                     bool flag_output_points, bool flag_output_uvs,
                     container_output_attributes &out_vertices_attributes,
                     container_output_indices &out_indices,
                     triangles::indices & out_indices_type,
                     number2 tex_left=number2(0), number2 tex_top=number2(1),
                     number2 tex_right=number2(1), number2 tex_bottom=number2(0)) {
            auto du = number2(1) / number2(uSamples - 1);
            auto dv = number2(1) / number2(vSamples - 1);

            number2 factor_remap_u = (tex_right - tex_left);
            number2 factor_remap_v = (tex_bottom-tex_top);

            out_indices_type = microtess::triangles::indices::TRIANGLES_STRIP;

            // iterates top to bottom and left to right
            // use the parametric time value 0 to 1
            for(index i=0; i < uSamples; i++) { // left to right
                number2 u = number2(i) * du;
                for(index j=0; j < vSamples; j++) { // top to bottom
                    // calculate the parametric v value
                    number2 v = number2(j) * dv;

                    if(flag_output_points) {
                        // calculate the point on the surface
                        number1 p[channels];
                        // compile time branching
                        if(type==patch_type::BI_QUADRATIC)
                            evaluateBiQuadraticSurface(u,v, meshPoints, channels, p);
                        else if (type==patch_type::BI_CUBIC)
                            evaluateBiCubicSurface(u,v, meshPoints, channels, p);
                        else return 0;
                        //int idx = i * vSamples + j;
                        for (int jx = 0; jx < channels; ++jx)
                            out_vertices_attributes.push_back(p[jx]);
                    }

                    // we need to separate this into another buffer, because number1!=number2 always
                    if(flag_output_uvs) {
                        out_vertices_attributes.push_back(number1(tex_left + u*factor_remap_u));
                        out_vertices_attributes.push_back(number1(tex_top + v*factor_remap_v));
                    }

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
            int window_size = (flag_output_points ? channels : 0) + (flag_output_uvs ? 2 : 0);
            return window_size;
        }

        /**
         * evaluate one point by 2d uv location
         * @param u u coord
         * @param v v coord
         * @param meshPoints array of size [4 * 4 * channels]
         * @param channels how many channels/coords in point
         * @param result array of size channels
         * @return
         */
        static
        number1 * evaluateBiCubicSurface(number1 u, number1 v,
                                       const number1 *meshPoints,
                                       unsigned int channels,
                                       number1 * result) {
            number1 temp[4][channels];

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
                // start of ix row in the flattened array
                index row = ix*4*channels;
                for (index jx = 0; jx < channels; ++jx) {
                    // go over all points in a given row
                    index p_1=row, p_2=p_1+channels, p_3=p_2+channels, p_4=p_3+channels;
                    temp[ix][jx] = b0*meshPoints[p_1+jx] + b1*meshPoints[p_2+jx] + b2*meshPoints[p_3+jx] + b3*meshPoints[p_4+jx];
                }
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
            for (index jx = 0; jx < channels; ++jx)
                result[jx] = b0*temp[0][jx] + b1*temp[1][jx] + b2*temp[2][jx] + b3*temp[3][jx];

            return result;
        };

        /**
         * evaluate one point by 2d uv location
         * @param u u coord
         * @param v v coord
         * @param meshPoints array of size [3 * 3 * channels]
         * @param channels how many channels/coords in point
         * @param result output array of size channels
         * @return result
         */
        static
        number1 * evaluateBiQuadraticSurface(number1 u, number1 v,
                                           const number1 *meshPoints,
                                           unsigned int channels,
                                           float * result) {
            number1 temp[3][channels];

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
                index row=ix*3*channels;
                for (index jx = 0; jx < channels; ++jx) {
                    // go over all points in a given row
                    index p_1=row, p_2=p_1+channels, p_3=p_2+channels;
                    temp[ix][jx] = b0*meshPoints[p_1+jx] + b1*meshPoints[p_2+jx] + b2*meshPoints[p_3+jx];
                }
            }

            t = v;
            it = number1(1) - t;
            b0 = it*it;
            b1 = number1(2) * t * it;
            b2 = t*t;

            // having got 3 points, we can use it as a bezier curve
            // to calculate the v direction. This should give us our
            // final point
            for (index jx = 0; jx < channels; ++jx)
                result[jx] = b0*temp[0][jx] + b1*temp[1][jx] + b2*temp[2][jx];

            return result;
        };
    };
}
