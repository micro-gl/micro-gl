#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/TriangleIndices.h>
#include <microgl/linked_list.h>
#include <microgl/array_container.h>
#include <stdexcept>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class EarClippingTriangulation {
    public:

        explicit EarClippingTriangulation(bool DEBUG = false);

        void compute(vec2_f * $pts,
                        index size,
                        array_container<index> & indices_buffer_triangulation,
                        array_container<triangles::boundary_info> * boundary_buffer,
                        const triangles::TrianglesIndices &requested =
                                        triangles::TrianglesIndices::TRIANGLES
        );

        void compute(vec2_32i * $pts,
                        index size,
                        array_container<index> & indices_buffer_triangulation,
                        array_container<triangles::boundary_info> * boundary_buffer,
                        const triangles::TrianglesIndices &requested =
                                    triangles::TrianglesIndices::TRIANGLES
                        );

        static index required_indices_size(index polygon_size,
                                           const triangles::TrianglesIndices &requested =
                                                        triangles::TrianglesIndices::TRIANGLES);

    private:
        struct NodeData {
            vec2_32i * pt;
            index original_index;
        };

        using LinkedList = linked_list<NodeData>;
        using Node = LinkedList::node_t;

        // t
        // positive if CCW
        static long long orientation_value(const Node * i,
                                           const Node * j,
                                           const Node * k);

        static int neighborhood_orientation_sign(const Node * v);

        // tv
        static char sign_orientation_value(const Node * i, const Node * j, const Node * k);

        // main

        static Node * maximal_y_element(const LinkedList * list);

        static bool isConvex(const Node * v, const LinkedList * list);

        static bool isEmpty(const Node * v, const LinkedList * list);

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop