#pragma once

#include "microgl/tesselation/vec2.h"
#include "microgl/tesselation/triangles.h"
#include <microgl/dynamic_array.h>

namespace microgl {

    namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
        using index = unsigned int;

        /**
         * todo:: we can:
         *
         * 1. make this O(n^2) by precomputing if a vertex is an ear, and every time we remove
         *    an ear, only recompute for it's two adjescent vertices.
         * 2. make it an O(r*n) algorithm where r are the number of reflex(concave) vertices,
         *    simply track them, and when testing for earness of a vertex, compare it against
         *    these reflex vertices.
         * 3. right now, the algorithm can tolerate small degenerate cases
         * 4. the hole punching technique might not work because of bad connections into
         *    the wrong side of another bridge. we can use heuristics to overcome some of these,
         *    but the better solution is much more work to do, which is a bummer :(
         */
        template<typename number>
        class ear_clipping_triangulation {
        public:
            static const int SUCCEED = 0;
            static const int ERR_HOLES_MUST_BE_OPERATED_WITH_RESULT_BUFFER = 1;
            using vertex = microgl::vec2<number>;

            enum class node_type_t {
                outer, inner, bridge
            };

            struct node_t {
                const vertex *pt = nullptr;
                index original_index = -1;
                node_t *prev = nullptr;
                node_t *next = nullptr;
                node_type_t type = node_type_t::outer;
            };

            struct hole {
                vertex *points = nullptr;
                index offset = 0, size = 0;
                bool orients_like_parent = false;
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

            struct poly_context_t {
                node_t *polygon = nullptr;
                node_t *left_most = nullptr;
                index size = 0;
            };

        public:
            static int compute(const vertex *$pts,
                               index size,
                               dynamic_array<index> &indices_buffer_triangulation,
                               const microgl::triangles::indices &requested,
                               dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                               dynamic_array<hole> *holes,
                               dynamic_array<vertex> *result);

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

            static index required_indices_size(index polygon_size,
                                               const microgl::triangles::indices &requested =
                                               microgl::triangles::indices::TRIANGLES);

        private:

            static
            node_t *polygon_to_linked_list(const vertex *$pts,
                                           index offset,
                                           index size,
                                           const node_type_t &type,
                                           bool reverse,
                                           pool_nodes_t &
            );

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

            static bool isEmpty(const node_t *v, node_t *list);

            static
            bool areEqual(const node_t *a,
                          const node_t *b);

            static
            bool isDegenrate(const node_t *v);

            static
            bool test_intersect(const vertex &a, const vertex &b, const vertex &c, const vertex &d,
                                vertex &st,
                                bool interior_only = false);

            static
            node_t *find_left_bottom_most_vertex(node_t *poly);

            static
            bool
            is_bbox_overlaps_axis(const vertex &a, const vertex &b, const vertex &c, const vertex &d, bool compare_x,
                                  bool interior_only = false);

            static
            bool is_bbox_overlaps(const vertex &a, const vertex &b, const vertex &c, const vertex &d,
                                  bool interior_only = false);

            static
            node_t *find_mutually_visible_vertex(node_t *poly, const vertex &v);

            static
            void merge_hole(node_t *outer, node_t *inner, node_t *inner_left_most_node, pool_nodes_t &pool);

            static
            int compare_poly_contexts(const void *a, const void *b, void *ctx);

            static
            bool business(const node_t *v, const vertex &a, const vertex &b, node_t *list);

            static
            bool inCone(const vertex &a0, const vertex &a, const vertex &a1, const vertex &b);
        };

    }

}
#include "ear_clipping_triangulation.cpp"
