#include <microgl/tesselation/complex_poly_tess.h>

namespace tessellation {

    bool test_intersect(const vertex &a, const vertex &b,
                        const vertex &c, const vertex &d) {
        auto ab = b - a;
        auto cd = d - c;
        auto ca = a - c;

        auto ab_cd = ab.x * cd.y - cd.x * ab.y;
        auto s = (ab.x * ca.y - ab.y * ca.x);
        auto t = (cd.x * ca.y - cd.y * ca.x);

        bool test = s >= 0 && s <= ab_cd && t >= 0 && t <= ab_cd;

        return test;
    }

    int find_mutually_visible_vertex_in_polygon(const vertex & main_vertex,
                                                vertex * poly_2, int size_2) {

        for (int ix = 0; ix < size_2; ++ix) {
            vertex against = poly_2[ix];
            bool fails = false;

            for (int jx = 0; jx < size_2; ++jx) {
                // (main_vertex, against)
                if(test_intersect(main_vertex,
                                  against,
                                  poly_2[jx],
                                  poly_2[(jx+1)%size_2])) {
                    fails = true;
                    break;
                }

            }

            if(!fails) {
                return ix;
            }

        }

        return -1;
    }

    void complex_poly_tess::compute(chunker<microgl::vec2_f> &pieces,
                                    dynamic_array<microgl::vec2_f> &result,
                                    dynamic_array<microgl::vec2_f> &indices) {

        using tree_t = tessellation::simple_components_tree::tree;
        using node_t = tree_t::node;
        using node_type_t = tree_t::node_type ;

        tree_t tree;
        tessellation::simple_components_tree::compute(
                pieces,
                tree);

//        dynamic_array<node_t *> stack;
//        stack.push_back(tree.root);
//        // group outer polygon with it's inner polygons (holes) and send
//        // to tesselator
//        while (!stack.empty()) {
//            auto * back = stack.back();
//
//            if(back->type==tree_t::node_type::)
//
//        }

        // go over the tree and extract fill nodes with holes
        dynamic_array<int> locations;
        for (index ix = 0; ix < tree.nodes_count; ++ix) {
            auto n = tree.nodes[ix];
            auto type = n.type;
            // unknown node types are nodes that have been compressed
            if(type==node_type_t::unknown || type==node_type_t::hole)
                continue;

            // we are on a fill node, it's children if any must be holes
            locations.push_back(n.index_poly);

            const auto children_count = n.children.size();

            // find hole children, as a matter of fact these should
            // be it's only children. note, children may be null
            for (index jx = 0; jx < children_count; ++jx) {
                auto child = n.children[jx];
                if(child && child->type==node_type_t::hole)
                    locations.push_back(child->index_poly);
            }

            // here tesselate the locations
        }

    }

}
