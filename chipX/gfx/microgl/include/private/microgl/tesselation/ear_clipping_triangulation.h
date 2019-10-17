#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using vertex = microgl::vec2_f;

    class ear_clipping_triangulation {
    public:
        static const int SUCCEED = 0;
        static const int ERR_HOLES_MUST_BE_OPERATED_WITH_RESULT_BUFFER = 1;

        struct node_t {
            vertex * pt= nullptr;
            index original_index=-1;
            node_t * prev = nullptr;
            node_t * next = nullptr;
        };

        struct pool_nodes_t {
            explicit pool_nodes_t(index count) {
                pool = new node_t[count];
            }
            ~pool_nodes_t() {
                delete [] pool;
            }
            node_t * get() {
                return &pool[_current++];
            }
        private:
            index _current=0;
            node_t *pool= nullptr;
        };

        struct poly_context_t {
            node_t *polygon= nullptr;
            node_t *left_most= nullptr;
            index size=0;
        };

        struct hole {
            vertex * points= nullptr;
            index offset=0, size=0;
        };

        static int compute(vertex *$pts,
                            index size,
                            dynamic_array<index> & indices_buffer_triangulation,
                            const microgl::triangles::TrianglesIndices &requested,
                            dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                            dynamic_array<hole> * holes,
                            dynamic_array<vertex> * result);

        static void compute(vertex *polygon,
                            index size,
                            dynamic_array<index> & indices_buffer_triangulation,
                            dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                            const microgl::triangles::TrianglesIndices &requested =
                            microgl::triangles::TrianglesIndices::TRIANGLES
        );

        static void compute(node_t *list,
                            index size,
                            dynamic_array<index> & indices_buffer_triangulation,
                            dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                            const microgl::triangles::TrianglesIndices &requested =
                            microgl::triangles::TrianglesIndices::TRIANGLES
        );

        static index required_indices_size(index polygon_size,
                                           const microgl::triangles::TrianglesIndices &requested =
                                           microgl::triangles::TrianglesIndices::TRIANGLES);


    private:

        static
        node_t * polygon_to_linked_list(vertex *$pts,
                                            index offset,
                                            index size,
                                            pool_nodes_t &
                                            );

        // t
        // positive if CCW
        static long long orientation_value(const node_t * i,
                                           const node_t * j,
                                           const node_t * k);

        static int neighborhood_orientation_sign(const node_t * v);

        // tv
        static char sign_orientation_value(const node_t * i, const node_t * j, const node_t * k);

        // main

        static node_t * maximal_y_element(node_t *list);

        static bool isConvex(const node_t * v, node_t *list);

        static bool isEmpty(const node_t * v, node_t *list);

        static
        bool areEqual(const node_t *a,
                      const node_t *b);

        static
        bool isDegenrate(const node_t *v);

        bool _DEBUG = false;
    };


}
