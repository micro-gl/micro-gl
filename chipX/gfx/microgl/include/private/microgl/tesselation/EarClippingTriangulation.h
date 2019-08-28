#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/TriangleIndices.h>
#include <microgl/linked_list.h>
#include <iostream>
#include <stdexcept>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;

    class EarClippingTriangulation {
    public:

        explicit EarClippingTriangulation(bool DEBUG = false);;

        index * compute(vec2_f * $pts,
                        index size,
                        index *indices_buffer_triangulation,
                        index indices_buffer_size,
                        const triangles::TrianglesIndices &requested
        );

        index * compute(vec2_32i * $pts,
                        index size,
                        index *indices_buffer_triangulation,
                        index indices_buffer_size,
                        const triangles::TrianglesIndices &requested
                        );


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

        bool isConvex(const Node * v, const LinkedList * list);

        static bool isEmpty(const Node * v, const LinkedList * list);

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop