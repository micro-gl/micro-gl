#pragma once

#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>

namespace microgl {
    namespace tessellation {

        template <typename number>
        class bezier_patch_tesselator {
            using index = unsigned int;
            using vertex3 = vec3<number>;
        public:

            static const char BI_QUADRATIC = 0;
            static const char BI_CUBIC = 1;

            static
            void compute(const vertex3 *meshPoints,
                         const index U, const index V,
                         const index uSamples, const index vSamples,
                         dynamic_array<number> &out_vertices_attributes,
                         dynamic_array<index> &out_indices,
                         triangles::indices & out_indices_type,
                         number tex_left=number(0), number tex_top=number(0),
                         number tex_right=number(1), number tex_bottom=number(1)
            ) {
                bool supported= U==V && (U==3 || U==4);
                if(!supported) return;
                const char type= U==3 ? BI_QUADRATIC : BI_CUBIC;

                auto du = number(1)/(uSamples-1);
                auto dv = number(1)/(vSamples-1);

                auto factor_remap_u = (tex_right - tex_left);
                auto factor_remap_v = (tex_top - tex_bottom);

                // iterates top to bottom and left to right
                // use the parametric time value 0 to 1
                for(index i=0; i < uSamples; i++) { // left to right
                    auto u = number(i) * du;
                    for(index j=0; j < vSamples; j++) { // top to bottom
                        // calculate the parametric v value
                        auto v = number(j) * dv;
                        // calculate the point on the surface
                        vertex3 p;
                        if(type==BI_QUADRATIC)
                            p = evaluateBiQuadraticSurface(u,v, meshPoints);
                        else if (type==BI_CUBIC)
                            p = evaluateBiCubicSurface(u,v, meshPoints);
                        else return;

                        int idx = i * vSamples + j;
                        out_vertices_attributes.push_back(p.x);
                        out_vertices_attributes.push_back(p.y);
                        out_vertices_attributes.push_back(p.z);
                        out_vertices_attributes.push_back(tex_left + u*factor_remap_u);
                        out_vertices_attributes.push_back(tex_bottom + v*factor_remap_v);

//            cout << to_string(p.x) + ", " + to_string(p.y) + ", " + to_string(u) + ", " + to_string(v) << endl;

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
            vertex3 evaluateBiCubicSurface(number u, number v, const vertex3 *meshPoints
//                                           vertex3 (*meshPoints)[4],
                                           ) {
                vertex3 temp[4], result;

                // 3rd degree bernstein polynomials coefficients
                // the t value inverted
                number t = u;
                number it = number(1)-t;
                number b0 = it*it*it;
                number b1 = number(3)*t*it*it;
                number b2 = number(3)*t*t*it;
                number b3 =  t*t*t;

                // first of all we will need to evaluate 4 curves in the u
                // direction. The points from those will be stored in this
                // temporary array
                for (index ix = 0; ix < 4; ++ix) {
                    index idx=ix*4;
                    temp[ix].x = b0*meshPoints[idx+0].x + b1*meshPoints[idx+1].x + b2*meshPoints[idx+2].x + b3*meshPoints[idx+3].x;
                    temp[ix].y = b0*meshPoints[idx+0].y + b1*meshPoints[idx+1].y + b2*meshPoints[idx+2].y + b3*meshPoints[idx+3].y;
                    temp[ix].z = b0*meshPoints[idx+0].z + b1*meshPoints[idx+1].z + b2*meshPoints[idx+2].z + b3*meshPoints[idx+3].z;
                }

                t = v;
                it = number(1)-t;
                b0 = it*it*it;
                b1 = number(3)*t*it*it;
                b2 = number(3)*t*t*it;
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
            vertex3 evaluateBiQuadraticSurface(number u, number v, const vertex3 *meshPoints) {
                vertex3 temp[3], result;

                // 2rd degree bernstein polynomials coefficients
                // the t value inverted
                number t = u;
                number it = number(1)-t;
                number b0 = it*it;
                number b1 = number(2)*t*it;
                number b2 = t*t;

                // first of all we will need to evaluate 4 curves in the u
                // direction. The points from those will be stored in this
                // temporary array
                for (index ix = 0; ix < 3; ++ix) {
                    index idx=ix*3;
                    temp[ix].x = b0*meshPoints[idx+0].x + b1*meshPoints[idx+1].x + b2*meshPoints[idx+2].x;
                    temp[ix].y = b0*meshPoints[idx+0].y + b1*meshPoints[idx+1].y + b2*meshPoints[idx+2].y;
                    temp[ix].z = b0*meshPoints[idx+0].z + b1*meshPoints[idx+1].z + b2*meshPoints[idx+2].z;
                }

                t = v;
                it = number(1)-t;
                b0 = it*it;
                b1 = number(2)*t*it;
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

}
