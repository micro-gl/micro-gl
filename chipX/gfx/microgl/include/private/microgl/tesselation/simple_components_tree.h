#pragma once

#include <microgl/tesselation/simplify_components.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;

    template <typename number>
    class simple_components_tree {
    public:
        using vertex = microgl::vec2<number>;

        enum class fill_rule {
            non_zero, even_odd
        };

        enum class orientation_t {
            cw, ccw, unknown
        };

        struct tree {
        public:
            enum class node_type {
                fill, hole, unknown
            };

            struct node {
                // may have missing null children, that were removed
                dynamic_array<node *> children;
                int index_poly = -1;
                int accumulated_winding = 0;
                node_type type = node_type::unknown;
                orientation_t orientation= orientation_t::unknown;
            };

            node * nodes = nullptr;
            node * root = nullptr;
            index nodes_count = 0;
            chunker<vertex> pieces;
            fill_rule rule=fill_rule::non_zero;

            ~tree() {
                if(nodes)
                    delete [] nodes;
                nodes= nullptr;
            }
        };

        static
        void compute(chunker<vertex> & pieces,
                     tree & tree,
                     fill_rule rule=fill_rule::non_zero
                     );

    private:

        static typename tree::node_type
        classify_fill_status(int accumulated_winding, const fill_rule & rule);

        static void
        compute_component_tree(chunker<vertex> &components, tree &tree);

        static
        void tag_and_merge(typename tree::node *root, const fill_rule & rule);

        static
        void compute_component_tree_recurse(typename tree::node *root,
                typename tree::node *current, chunker<vertex> &components);

        static int
        compare_simple_non_intersecting_polygons(vertex *poly_1, index size_1, bool poly_1_CCW, vertex *poly_2,
                                                 index size_2,
                                                 bool poly_2_CCW);

        static vertex
        find_point_in_simple_polygon_interior(vertex *poly, int size, bool CCW=true);

        static
        simple_components_tree::orientation_t compute_polygon_direction(vertex *poly, int size);

        static
        int find_next_unique_vertex(int idx, vertex *poly, int size);

        static
        int find_left_bottom_most_vertex(vertex *poly, int size);

        static
        bool point_inside_convex_poly_interior(const vertex &point, const vertex *poly, int size, bool CCW=true);

        static
        bool point_inside_simple_polygon_cn(const vertex &point, const vertex *poly, int size);

        static
        int point_inside_simple_polygon_wn(const vertex &point, const vertex *poly, int size);

        static
        int isLeft(const vertex &point, const vertex &a, const vertex &b);

        static
        int classify_point(const vertex &point, const vertex &a, const vertex &b);
    };


}

#include "../../src/simple_components_tree.cpp"