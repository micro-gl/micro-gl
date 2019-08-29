#include <microgl/tesselation/EarClippingTriangulation.h>

namespace tessellation {

    EarClippingTriangulation::EarClippingTriangulation(bool DEBUG) :
            _DEBUG{DEBUG} {};

    index *EarClippingTriangulation::compute(microgl::vec2_f *$pts, index size,
                                             index *indices_buffer_triangulation,
                                             index indices_buffer_size,
                                             const triangles::TrianglesIndices &requested) {
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

    index *EarClippingTriangulation::compute(microgl::vec2_32i *$pts,
                                             index size,
                                             index *indices_buffer_triangulation,
                                             index indices_buffer_size,
                                             const triangles::TrianglesIndices &requested) {

        if(requested==triangles::TrianglesIndices::TRIANGLES) {
            if(required_indices_size(size, requested) > indices_buffer_size)
                throw std::runtime_error("size of the indices buffer is "
                                         "not enough for TRIANGLES !!!");
        }
        else if(requested==triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY) {
            if(required_indices_size(size, requested) > indices_buffer_size)
                throw std::runtime_error("size of the indices buffer is not enough "
                                         "for TRIANGLES_WITH_BOUNDARY !!!");
        }
        else
            throw std::runtime_error("only TRIANGLES and TRIANGLES_WITH_BOUNDARY "
                                     "are supported !!!");


        bool requested_triangles_with_boundary =
                requested==triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY;
        auto *indices = indices_buffer_triangulation;

        // create a linked list with static memory on the stack :)
        // lets also make heap version
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


    long long
    EarClippingTriangulation::orientation_value(const EarClippingTriangulation::Node *i,
                                                const EarClippingTriangulation::Node *j,
                                                const EarClippingTriangulation::Node *k) {
        return i->data.pt->x * (j->data.pt->y - k->data.pt->y) +
               j->data.pt->x * (k->data.pt->y - i->data.pt->y) +
               k->data.pt->x * (i->data.pt->y - j->data.pt->y);
    }

    int EarClippingTriangulation::neighborhood_orientation_sign(
            const EarClippingTriangulation::Node *v) {
        const Node * l = v->predecessor();
        const Node * r = v->successor();

        // pay attention that this can return 0, although in the algorithm
        // it does not return 0 never here.
        return sign_orientation_value(l, v, r) > 0 ? 1 : -1;
    }

    char
    EarClippingTriangulation::sign_orientation_value(const EarClippingTriangulation::Node *i,
                                                     const EarClippingTriangulation::Node *j,
                                                     const EarClippingTriangulation::Node *k) {
        auto v = orientation_value(i, j, k);

        // we clip, to avoid overflows down the road
        if(v > 0)
            return 1;
        else if(v < 0)
            return -1;
        else
            return 0;
    }

    EarClippingTriangulation::Node *EarClippingTriangulation::maximal_y_element(
                                const EarClippingTriangulation::LinkedList *list) {
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

    bool EarClippingTriangulation::isConvex(const EarClippingTriangulation::Node *v,
                                            const EarClippingTriangulation::LinkedList *list) {
        // the maximal y elemnt is always convex, therefore if
        // they have the same orientation, then v is also convex
        return neighborhood_orientation_sign(v) *
               neighborhood_orientation_sign(maximal_y_element(list)) > 0;
    }

    bool EarClippingTriangulation::isEmpty(const EarClippingTriangulation::Node *v,
                                           const EarClippingTriangulation::LinkedList *list) {
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

    index EarClippingTriangulation::required_indices_size(const index polygon_size,
                                                          const triangles::TrianglesIndices &requested) {
        switch (requested) {
            case triangles::TrianglesIndices::TRIANGLES:
                return (polygon_size - 2)*3;
            case triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY:
                return (polygon_size - 2)*4;
            default:
                return 0;
        }

    }

}
