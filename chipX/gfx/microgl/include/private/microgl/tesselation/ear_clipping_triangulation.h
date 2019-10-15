#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/linked_list.h>
#include <microgl/chunker.h>
//#include <microgl/dynamic_array.h>
//#include <microgl/array_container.h>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;
    using namespace microgl;
    using vertex = microgl::vec2_f;

    class ear_clipping_triangulation {
    public:
        struct hole {
            vec2_f * points;
            index offset, size;
        };

        explicit ear_clipping_triangulation(bool DEBUG = false);

        static void compute(microgl::vec2_f *$pts,
                            index size,
                            dynamic_array<index> & indices_buffer_triangulation,
                            dynamic_array<triangles::boundary_info> * boundary_buffer,
                            dynamic_array<hole> * holes,
                            dynamic_array<vec2_f> * result,
                            const triangles::TrianglesIndices &requested);

        static void compute(vec2_32i * $pts,
                        index size,
                            dynamic_array<index> & indices_buffer_triangulation,
                            dynamic_array<triangles::boundary_info> * boundary_buffer,
                        const triangles::TrianglesIndices &requested =
                                    triangles::TrianglesIndices::TRIANGLES
                        );

        static index required_indices_size(index polygon_size,
                                           const triangles::TrianglesIndices &requested =
                                                        triangles::TrianglesIndices::TRIANGLES);

        struct NodeData {
            vec2_f * pt;
            index original_index;
        };

        using LinkedList = linked_list<NodeData>;
        using Node = LinkedList::node_t;

    private:

        static
        LinkedList * polygon_to_linked_list(vertex *$pts,
                                            index offset,
                                            index size
                                            );

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

        static
        bool areEqual(const ear_clipping_triangulation::Node *a,
                      const ear_clipping_triangulation::Node *b);

        static
        bool isDegenrate(const ear_clipping_triangulation::Node *v);

        bool _DEBUG = false;
    };


}
