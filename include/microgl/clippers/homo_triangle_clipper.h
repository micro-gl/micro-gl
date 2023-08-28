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

#include "../math/vertex4.h"
#include "../traits.h"

namespace microgl {
    namespace clipping {

        template<typename number>
        class homo_triangle_clipper {
        public:
            using const_ref = const number &;
            using vertex4 = microgl::vertex4<number>;
            using vertex4_const_ref = const vertex4 &;

            struct super_vertex {
                // barycentric weights in original triangle
                vertex4 bary;
                vertex4 point;
            };

            template <unsigned N>
            struct typed_vertices_list {
            private:
                super_vertex _vertices[N];
                unsigned _count=0;
            public:
                unsigned size() { return _count; }
                void push_back(const super_vertex& v) {
                    _vertices[_count++]=v;
                }
                super_vertex &operator[](unsigned index) {
                    return _vertices[index];
                }
                typed_vertices_list & operator=(typed_vertices_list && list) noexcept {
                    clear();
                    const auto size_= list.size();
                    for (unsigned ix = 0; ix < size_; ++ix)
                        push_back(list[ix]);
                    list.clear();
                    return *this;
                }
                void clear() { _count=0; }
            };

            // maximum of 9 vertices against max 6 clipping planes,
            // better put it on the stack then allocate from_sampler the heap
            using vertices_list= typed_vertices_list<9>;

            static bool compute(vertex4_const_ref $v0, vertex4_const_ref $v1,
                                vertex4_const_ref $v2, vertices_list &out_list) {
                super_vertex v0, v1, v2;
                // convert input vertices into our own representation
                v0.point= $v0; v0.bary= {1,0,0,1};
                v1.point= $v1; v1.bary= {0,1,0,1};
                v2.point= $v2; v2.bary= {0,0,1,1};

                /*
                clipping planes defined in Clip space, the 4D space in which the vertices are expressed before the
                perspective divide is performed.
                A 4D plane P=(a, b, c, d, e) is the set of points (x, y, z, w) that satisfy the equation:
                ax + by + cz + dw + e = 0
                Because all the clipping planes cross the origin (i.e. e = 0), a plane can be represented with a 4D vector, and
                a point can be classified against a plane using the dot product:
                dot((a, b, c, d),(x, y, z, w)) = ax + by + cz + dw + 0 = ax + by + cz + dw = 0
                */
                number zero{0}, one{1};
                const vertex4 planes[] = {
                    vertex4{ 1,  0,  0, 1}, // left plane,      x+w >= 0
                    vertex4{-1,  0,  0, 1}, // right plane,     w-x >= 0
                    vertex4{ 0,  1,  0, 1}, // bottom plane,    y+w >= 0
                    vertex4{ 0, -1,  0, 1}, // top plane,       w-y >= 0
                    vertex4{ 0,  0,  1, 1}, // near plane,      z+w >= 0, assume positive z is pointing away from us
                    vertex4{ 0,  0, -1, 1}  // far plane,       w-z >= 0
                };

                vertices_list in_list;

                /*
                iterate through the clipping planes. for each clip plane, inList is used as input and outList as output. the two are then
                swapped before considering the next plane.
                because the lists' contents are swapped	at the beginning of the main loop, the initial indices are placed in outList.
                */
                in_list.clear(); out_list.clear();
                out_list.push_back(v0);
                out_list.push_back(v1);
                out_list.push_back(v2);

                constexpr unsigned planeCount = sizeof(planes) / sizeof(planes[0]);
                //clipping a convex polygon with k vertices against a plane yields a convex polygon with k+1 vertices
                constexpr unsigned MAX_VERTICES = 3 + planeCount;
                // product and class vectors
                bool plane_vertices_classifications[MAX_VERTICES];
                number plane_vertices_product[MAX_VERTICES];

                for (unsigned ix=0; ix<planeCount; ix++) {
                    unsigned vertex_count = out_list.size();
                    if (vertex_count < 2) break; // todo: why 2?
                    in_list = microgl::traits::move(out_list); // move everything from_sampler out to in
                    const vertex4 &plane = planes[ix];
                    bool cull = true;

                    /*
                    test_texture all vertices against the plane, i.e. find out for each vertex, in which side of the plane lies.
                    this is done using the dot product.
                    */
                    for (unsigned kx=0; kx<vertex_count; kx++) {
                        // dot product plane with point, this gives classification
                        const number product = plane.dot(in_list[kx].point);
                        const bool inside = product>=number{0};
                        plane_vertices_product[kx] = product;
                        plane_vertices_classifications[kx] = inside;
                        cull = cull && !inside;
                    }
                    // if all vertices are outside a plane, then bail out early
                    if (cull) break;

                    /*
                    consider all triangle's edges, four cases are possible depending on which side the edge's vertices are:

                    first case : first outside and second inside
                    following the edge we're moving inside the volume.
                    replace the first vertex with the intersection between the edge and the clip volume; i.e. add the new vertex and
                    the second one to the output list.
                    second case : first inside and second outside
                    following the edge we're moving outside the volume.
                    replace the second vertex with the intersection between the edge and the clip volume; i.e. add the new vertex
                    to the output list.
                    third case : both outside
                    do nothing

                    fourth case : both inside
                    add the second vertex to the output list
                    */

                    for (unsigned jx=0; jx<vertex_count; jx++) {
                        unsigned first_index= jx, second_index= (jx+1)%vertex_count;
                        bool first_inside = plane_vertices_classifications[first_index];
                        bool second_inside = plane_vertices_classifications[second_index];
                        auto & first_vertex= in_list[first_index];
                        auto & second_vertex= in_list[second_index];

                        //first or second case?
                        if (first_inside!=second_inside) {
                            const number t = intersectEdgePlane(plane_vertices_product[first_index],
                                    plane_vertices_product[second_index]);
                            super_vertex new_vertex= lerpSuperVertex(t, first_vertex, second_vertex);
                            out_list.push_back(new_vertex);
                            if (!first_inside) //cover first case
                                out_list.push_back(second_vertex);
                        }
                        else if (first_inside && second_inside) //fourth case
                            out_list.push_back(second_vertex);
                        //else third case
                    }
                }

                if (out_list.size() < 3) out_list.clear(); //discard triangle
                return out_list.size()!=0;
            }

        private:
            inline static number intersectEdgePlane(const number p1DotPlane, number p2DotPlane) {
                /*
                Precondition : sign(p1DotPlane) != sign(p2DotPlane)
                Following what have been said about clipping planes in the definition of SHClipper::clipTriangleTask,
                find intersection between an edge "(1-t)p1 + tp2, t in [0,1]" and a plane P = (a, b, c, d) in 4D, where
                p1 = (x1, y1, z1, w1) and p2 = (x2, y2, z2, w2) are the two end point of the edge.
                The instersection is the point p' = (1-t)p1 + tp2 = p1 + t(p2 - p1) that satisfy the following:
                dot(P, p') = 0.
                Substituting P and p' yields:
                dot(P, p') = (a, b, c, d)(p1 + t(p2 - p1))
                           = (ax1 + by1 + cz1 + dw1) + t( ax2 + by2 + cz2 + dw2 - (ax1 + by1 + cz1 + dw1)) = 0
                -> t = - (ax1 + by1 + cz1 + dw1) / ( ax2 + by2 + cz2 + dw2 - (ax1 + by1 + cz1 + dw1))
                     = - dot(P, p1) / (dot(P, p2) - dot(P, p1))
                and t is in [0, 1] if p' lies on the segment.
                */
                const auto v = -p1DotPlane;
                const auto v1 = p2DotPlane;
                return v / (v1 + v);
            }

            inline static super_vertex lerpSuperVertex(const number t, const super_vertex & v0,
                                                       const super_vertex & v1) {
                super_vertex result;
                result.point= v0.point + (v1.point-v0.point)*t;
                result.bary= v0.bary + (v1.bary-v0.bary)*t;
                return result;
            }
        };
    }
}