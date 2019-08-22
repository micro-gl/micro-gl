#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <algorithm>
#include <microgl/Types.h>
#include <microgl/TriangleIndices.h>
#include <vector>
#include <iostream>
#include <stdexcept>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
    using index = unsigned int;

    template<typename T>
    struct linkedlist_t {

    public:
        struct node_t {
            T data;
            node_t * prev = nullptr;
            node_t * next = nullptr;

            node_t* predecessor() const {
                return this->prev;
            }

            node_t* successor() const {
                return this->next;
            }
        };

        explicit linkedlist_t(bool CYCLIC = false) :
                                        _cyclic{CYCLIC} {};

        unsigned int size() const {
            return _size;
        }

        void clear() {
            while (size() != 0)
                removeFirst();
        }

        void addLast(node_t * node) {
            if(node==nullptr)
                return;

            if(last) {
                last->next = node;
                node->prev = last;
                last = node;
            } else
                first = last = node;

            _size+=1;

            handle_cyclic();
        }

        void resetNode(node_t * node) {
            node->prev = nullptr;
            node->next = nullptr;
        }

        void removeLast() {
            if(last==nullptr)
                return;

            if(last->prev) {
                last->prev->next = nullptr;
                last = last->prev;
            }

            _size-=1;

            handle_cyclic();
        }

        void addFirst(node_t* node) {
            if(node==nullptr)
                return;

            if(first) {
                node->next = first;
                first->prev = node;
                first = node;
            } else
                first = last = node;

            _size+=1;

            handle_cyclic();
        }

        void removeFirst() {
            if(first==nullptr)
                return;

            if(first->prev) {
                first->next->prev = nullptr;
                first = first->next;
            }

            _size-=1;

            handle_cyclic();
        }

        // we assume that node is part of the linked list
        void unlink(node_t * node) {
            if(!node)
                return;

            if(node==first)
                removeFirst();
            else if(node==last)
                removeLast();
            else {
                bool has_prev = node->prev;
                bool has_next = node->next;

                if(has_prev)
                    node->prev->next = node->next;

                if(has_next)
                    node->next->prev = node->prev;

                _size-=1;
            }

            handle_cyclic();
        }

        node_t * getFirst() const {
            return first;
        }

        node_t *getLast() const {
            return last;
        }

        bool isCyclic() const {
            return _cyclic;
        }

    private:
        unsigned int _size = 0;
        node_t * first = nullptr;
        node_t * last = nullptr;
        bool _cyclic = false;

        void handle_cyclic() {
            if(!isCyclic())
                return;

            if(first==last)
                return;

            first->prev = last;
            last->next = first;
        }

    };

    class EarClippingTriangulation {
    public:

        struct NodeData {
            vec2_32i * pt;
            index original_index;
        };

        using LinkedList = linkedlist_t<NodeData>;
        using Node = LinkedList::node_t;

        explicit EarClippingTriangulation(bool DEBUG = false) :
                                            _DEBUG{DEBUG} {};

        // t
        // positive if CCW
        long long orientation_value(const Node * i,
                                    const Node * j,
                                    const Node * k)
        {
            return i->data.pt->x * (j->data.pt->y - k->data.pt->y) +
                   j->data.pt->x * (k->data.pt->y - i->data.pt->y) +
                   k->data.pt->x * (i->data.pt->y - j->data.pt->y);
        }

        int neighborhood_orientation_sign(const Node * v)
        {
            const Node * l = v->predecessor();
            const Node * r = v->successor();

            // pay attention that this can return 0, although in the algorithm
            // it does not return 0 never here.
            return sign_orientation_value(l, v, r) > 0 ? 1 : -1;
        }

        // tv
        char sign_orientation_value(const Node * i, const Node * j, const Node * k) {
            auto v = orientation_value(i, j, k);

            // we clip, to avoid overflows down the road
            if(v > 0)
                return 1;
            else if(v < 0)
                return -1;
            else
                return 0;
        }

        // main

        Node * maximal_y_element(const LinkedList * list) {
            Node * first = list->getFirst();
            Node * maximal_index = first;
            Node * node = first;
            int maximal_y = first->data.pt->y;//pts[maximal_index].y;

            for (unsigned int ix = 0; ix < list->size(); ++ix) {
                if(node->data.pt->y > maximal_y) {
                    maximal_y = node->data.pt->y;
                    maximal_index = node;
                }

                node = node->successor();
            }

            return maximal_index;
        }

        bool isConvex(const Node * v, const LinkedList * list)
        {
            // the maximal y elemnt is always convex, therefore if
            // they have the same orientation, then v is also convex
            return neighborhood_orientation_sign(v) *
                    neighborhood_orientation_sign(maximal_y_element(list)) > 0;
        }

        bool isEmpty(const Node * v, const LinkedList * list)
        {
            int tsv;

            const Node * l = v->predecessor();
            const Node * r = v->successor();

            tsv = sign_orientation_value(v, l, r);

            Node * n  = list->getFirst();
            index ix = 0;

            do {

                if(n==v || n==l || n==r)
                    continue;

                if(tsv * sign_orientation_value(v, l, n)>=0 &&
                   tsv * sign_orientation_value(l, r, n)>=0 &&
                   tsv * sign_orientation_value(r, v, n)>=0
                   )
                    return false;

            } while(ix++<list->size() && (n=n->successor()));

            return true;
        }

#define abs(a) ((a)<0 ? -(a) : (a))

        index * compute(vec2_f * $pts,
                        index size,
                        index *indices_buffer_triangulation,
                        index indices_buffer_size,
                        const TrianglesIndices &requested
        ) {
            // I could have made a template for point types and
            // conserve stack memory, but the hell with it for now
            vec2_32i vertices_int[size];

            for (index ix = 0; ix < size; ++ix) {
                vertices_int[ix] = $pts[ix]<<5;
            }

            return compute(vertices_int, size,
                    indices_buffer_triangulation,
                    indices_buffer_size, requested);
        }

        index * compute(vec2_32i * $pts,
                        index size,
                        index *indices_buffer_triangulation,
                        index indices_buffer_size,
                        const TrianglesIndices &requested
                        ) {

            if(requested==TrianglesIndices::TRIANGLES) {
                if(3*(size - 2) != indices_buffer_size)
                    throw std::runtime_error("size of the indices buffer is "
                                             "not accurate for TRIANGLES !!!");
            }
            else if(requested==TrianglesIndices::TRIANGLES_WITH_BOUNDARY) {
                if(3*(size - 2) + (size - 2) != indices_buffer_size)
                    throw std::runtime_error("size of the indices buffer is not accurate "
                                             "for TRIANGLES_WITH_BOUNDARY !!!");
            }
            else
                throw std::runtime_error("only TRIANGLES and TRIANGLES_WITH_BOUNDARY "
                                         "are supported !!!");


            bool requested_triangles_with_boundary =
                    requested==TrianglesIndices::TRIANGLES_WITH_BOUNDARY;
            auto *indices = indices_buffer_triangulation;

            // create a linked list with static memory on the stack :)
            LinkedList pts{true};
            Node nodes[size];

            for (index ix = 0; ix < size; ++ix) {
                nodes[ix].data = {&$pts[ix], ix};

                pts.addLast(&nodes[ix]);
            }

            // start algorithm

            index ind = 0;
            Node * point;

            for (index ix = 0; ix < size - 2; ++ix) {

                point = pts.getFirst();

                for (index jx = 0; jx < size; ++jx) {

                    if (isConvex(point, &pts) && isEmpty(point, &pts)) {

                        indices[ind + 0] = point->predecessor()->data.original_index;
                        indices[ind + 1] = point->data.original_index;
                        indices[ind + 2] = point->successor()->data.original_index;

                        // record boundary
                        if(requested_triangles_with_boundary) {
                            // classify if edges are on boundary
                            int first_edge_index_distance = abs((int)indices[ind + 0] - (int)indices[ind + 1]);
                            int second_edge_index_distance = abs((int)indices[ind + 1] - (int)indices[ind + 2]);
                            int third_edge_index_distance = abs((int)indices[ind + 2] - (int)indices[ind + 0]);

                            bool first = first_edge_index_distance==1 || first_edge_index_distance==size-1;
                            bool second = second_edge_index_distance==1 || second_edge_index_distance==size-1;
                            bool third = third_edge_index_distance==1 || third_edge_index_distance==size-1;

                            index info = triangles::create_boundary_info(first, second, third);

                            indices[ind + 3] = info;
                        }

                        ind += requested_triangles_with_boundary ? 4 : 3;

                        // prune the point from the polygon
                        pts.unlink(point);

                        break;
                    }

                    point = point->next;
                }

            }

            pts.clear();

            return indices_buffer_triangulation;
        }


    private:

        bool _DEBUG = false;
    };


}

#pragma clang diagnostic pop