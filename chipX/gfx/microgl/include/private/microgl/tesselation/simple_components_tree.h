#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/chunker.h>
#include <microgl/tesselation/simplify_components.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using vertex = microgl::vec2_f;

    enum class direction {
        cw, ccw, unknown
    };

    class simple_components_tree {
    public:

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
            };

            node * nodes = nullptr;
            node * root = nullptr;
            int nodes_count = 0;
            chunker<vertex> pieces;

            ~tree() {
                if(nodes)
                    delete [] nodes;
                nodes= nullptr;
            }
        };

        static
        void compute(chunker<vertex> & pieces,
                            tree & tree
                            );

    private:

    };


}

#pragma clang diagnostic pop