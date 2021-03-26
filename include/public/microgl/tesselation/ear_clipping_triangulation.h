#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>

namespace microgl {

    namespace tessellation {

#define abs_(a) ((a)<0 ? -(a) : (a))
        using index = unsigned int;

        /**
         * todo:: add templating on container so others can use vector<> for example
         *
         * 1. we make this O(n^2) by pre computing if a vertex is an ear, and every time we remove
         *    an ear, only recompute for it's two adjacent vertices.
         * 2. This algorithm was customized by me tolerate simple polygons with touching vertices on edges,
         *    which is important. It can also tolerate holes which is cool, BUT not opposing edges on edges
         *    in order to create perfect hole (this will not work due to aliasing).
         * 3. I also remove degenerate vertices as I go to make it easier on the algorithm.
         * 4. I can also make the sign function more robust but it really won't matter much.
         * 5. I can also make it an O(r*n) algorithm where r are the number of reflex(concave) vertices,
         *    simply track them, and when testing for earness of a vertex, compare it against
         *    these reflex vertices.
         * 6. The strength of this algorithm is it's simplicity, short code, stability, low memory usage,
         *    does not require crazy numeric robustness.
         */
        template<typename number, template<typename...> class container_type>
        class ear_clipping_triangulation {
        public:
            using vertex = microgl::vec2<number>;

            struct node_t {
                const vertex *pt = nullptr;
                index original_index = -1;
                node_t *prev = nullptr;
                node_t *next = nullptr;
                bool is_ear=false;
                bool isValid() {
                    return prev!= nullptr && next!= nullptr;
                }
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
                                container_type<index> &indices_buffer_triangulation,
                                container_type<microgl::triangles::boundary_info> *boundary_buffer,
                                microgl::triangles::indices &output_type);
        private:

            static void compute(node_t *list,
                                index size,
                                container_type<index> &indices_buffer_triangulation,
                                container_type<microgl::triangles::boundary_info> *boundary_buffer,
                                microgl::triangles::indices &output_type);

            static
            node_t *polygon_to_linked_list(const vertex *$pts,
                                           index offset,
                                           index size,
                                           bool reverse,
                                           pool_nodes_t &);
            static number orientation_value(const vertex *a, const vertex *b, const vertex *c);
            static int neighborhood_orientation_sign(const node_t *v);
            static char sign_orientation_value(const vertex *i, const vertex *j, const vertex *k);
            static node_t *maximal_y_element(node_t *list);
            static bool isEmpty(node_t *v);
            static bool areEqual(const node_t *a, const node_t *b);
            static bool isDegenerate(const node_t *v);
            static auto remove_degenerate_from(node_t *v, bool backwards) -> node_t *;
            static void update_ear_status(node_t *vertex, const int &polygon_orientation);

        };

    }

}
#include "ear_clipping_triangulation.cpp"
