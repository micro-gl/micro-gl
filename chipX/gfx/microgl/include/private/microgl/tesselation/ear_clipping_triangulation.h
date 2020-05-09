#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>

namespace microgl {

    namespace tessellation {

#define abs_(a) ((a)<0 ? -(a) : (a))
        using index = unsigned int;

        /**
         * todo:: we can:
         *
         * 1. make this O(n^2) by precomputing if a vertex is an ear, and every time we remove
         *    an ear, only recompute for it's two adjescent vertices.
         * 2. make it an O(r*n) algorithm where r are the number of reflex(concave) vertices,
         *    simply track them, and when testing for earness of a vertex, compare it against
         *    these reflex vertices.
         */
        template<typename number>
        class ear_clipping_triangulation {
        public:
            using vertex = microgl::vec2<number>;

            struct node_t {
                const vertex *pt = nullptr;
                index original_index = -1;
                node_t *prev = nullptr;
                node_t *next = nullptr;
            };

        private:

            struct pool_nodes_t {
                explicit pool_nodes_t(index count) {
                    pool = new node_t[count];
                }

                ~pool_nodes_t() {
                    delete[] pool;
                }

                node_t *get() {
                    return &pool[_current++];
                }

            private:
                index _current = 0;
                node_t *pool = nullptr;
            };

        public:
            static void compute(const vertex *polygon,
                                index size,
                                dynamic_array<index> &indices_buffer_triangulation,
                                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                                const microgl::triangles::indices &requested =
                                    microgl::triangles::indices::TRIANGLES
            );

            static void compute(node_t *list,
                                index size,
                                dynamic_array<index> &indices_buffer_triangulation,
                                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                                const microgl::triangles::indices &requested =
                                microgl::triangles::indices::TRIANGLES
            );

        private:

            static
            node_t *polygon_to_linked_list(const vertex *$pts,
                                           index offset,
                                           index size,
                                           bool reverse,
                                           pool_nodes_t &);

            // t
            // positive if CCW
            static number orientation_value(const vertex *a,
                                            const vertex *b,
                                            const vertex *c);

            static int neighborhood_orientation_sign(const node_t *v);

            // tv
            static char sign_orientation_value(const vertex *i, const vertex *j, const vertex *k);

            // main

            static node_t *maximal_y_element(node_t *list);

            static bool isConvex(const node_t *v, node_t *list);
            static bool isConvex(const node_t *v, int global_orientation);

            static bool isEmpty(const node_t *v, node_t *list);

            static
            bool areEqual(const node_t *a,
                          const node_t *b);

            static
            bool isDegenrate(const node_t *v);

        };

    }

}
#include "ear_clipping_triangulation.cpp"
