#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/dynamic_array.h>

namespace microgl {

    namespace tessellation {

#define abs_(a) ((a)<0 ? -(a) : (a))
        using index = unsigned int;

        template<typename number>
        class monotone_polygon_triangulation {
        public:
            using vertex = microgl::vec2<number>;

            enum class monotone_axis {
                x_monotone, y_monotone
            };

            enum class orientation {
                cw, ccw
            };

            struct node_t {
                const vertex *pt = nullptr;
                index original_index = -1;
                node_t *prev = nullptr;
                node_t *next = nullptr;
                char chain_index=0;
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
                                const monotone_axis & axis,
                                dynamic_array<index> &indices_buffer_triangulation,
                                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                                microgl::triangles::indices &output_type
            );


        private:
            static void
            compute(node_t *list, index size,
                    const monotone_axis & axis,
                    dynamic_array<index> & indices_buffer_triangulation,
                    dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                    microgl::triangles::indices &output_type);

            static
            node_t *polygon_to_linked_list(const vertex *$pts,
                                           index offset,
                                           index size,
                                           bool reverse,
                                           pool_nodes_t &);
            static void find_min_max(node_t *list, const monotone_axis &axis, node_t **min, node_t **max) ;
            static bool a_B_b(node_t *a, node_t *b, const monotone_axis & axis);
            static bool a_G_b(node_t *a, node_t *b, const monotone_axis & axis);
            static bool isDegenerate(const node_t *v);
            static int classify_point(const vertex & point, const vertex &a, const vertex & b);
        };

    }

}
#include "monotone_polygon_triangulation.cpp"
