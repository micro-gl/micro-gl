#include <microgl/tesselation/EarClippingTriangulation.h>

namespace tessellation {

    EarClippingTriangulation::EarClippingTriangulation(bool DEBUG) :
            _DEBUG{DEBUG} {};

    void EarClippingTriangulation::compute(microgl::vec2_f *$pts,
                                           index size,
                                           dynamic_array<index> & indices_buffer_triangulation,
                                           dynamic_array<triangles::boundary_info> * boundary_buffer,
                                           const triangles::TrianglesIndices &requested) {
        // I could have made a template for point types and
        // conserve stack memory, but the hell with it for now
        vec2_32i vertices_int[size];

        for (index ix = 0; ix < size; ++ix) {
            vertices_int[ix] = $pts[ix]<<0;
        }

        compute(vertices_int,
                size,
                indices_buffer_triangulation,
                boundary_buffer,
                requested);
    }

    void EarClippingTriangulation::compute(microgl::vec2_32i *$pts,
                                           index size,
                                           dynamic_array<index> & indices_buffer_triangulation,
                                           dynamic_array<triangles::boundary_info> * boundary_buffer,
                                           const triangles::TrianglesIndices &requested) {

        bool requested_triangles_with_boundary =
                requested==triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY;
        auto &indices = indices_buffer_triangulation;

        // create a linked list with static memory on the stack :)
        // lets also make heap version
        LinkedList pts{true};
        Node nodes[size];

        // todo:: filter out degenerate points, points at the same location
        for (index ix = 0; ix < size; ++ix) {
            nodes[ix].data = {&$pts[ix], ix};

            pts.addLast(&nodes[ix]);
        }

        // start algorithm

        index ind = 0;
        Node * point;

        for (index ix = 0; ix < size - 2; ++ix) {

            point = pts.getFirst();

//            for (index jx = 0; jx < size; ++jx) {
            for (index jx = 0; jx < pts.size(); ++jx) {

                if (isDegenrate(point) || (isConvex(point, &pts) && isEmpty(point, &pts))) {

                    indices.push_back(point->predecessor()->data.original_index);
                    indices.push_back(point->data.original_index);
                    indices.push_back(point->successor()->data.original_index);

                    // record boundary
                    if(requested_triangles_with_boundary) {
                        // classify if edges are on boundary
                        unsigned int first_edge_index_distance = abs((int)indices[ind + 0] - (int)indices[ind + 1]);
                        unsigned int second_edge_index_distance = abs((int)indices[ind + 1] - (int)indices[ind + 2]);
                        unsigned int third_edge_index_distance = abs((int)indices[ind + 2] - (int)indices[ind + 0]);

                        bool first = first_edge_index_distance==1 || first_edge_index_distance==size-1;
                        bool second = second_edge_index_distance==1 || second_edge_index_distance==size-1;
                        bool third = third_edge_index_distance==1 || third_edge_index_distance==size-1;

                        index info = triangles::create_boundary_info(first, second, third);

                        boundary_buffer->push_back(info);
                    }

                    ind += 3;

                    // prune the point from the polygon
                    pts.unlink(point);

                    break;
                }

                point = point->next;
            }

        }

        pts.clear();
    }

    long long
    EarClippingTriangulation::orientation_value(const EarClippingTriangulation::Node *i,
                                                const EarClippingTriangulation::Node *j,
                                                const EarClippingTriangulation::Node *k) {
        return i->data.pt->x * int64_t(j->data.pt->y - k->data.pt->y) +
               j->data.pt->x * int64_t(k->data.pt->y - i->data.pt->y) +
               k->data.pt->x * int64_t(i->data.pt->y - j->data.pt->y);
    }

    // ts
    int EarClippingTriangulation::neighborhood_orientation_sign(
            const EarClippingTriangulation::Node *v) {
        const Node * l = v->predecessor();
        const Node * r = v->successor();

        // pay attention that this can return 0, although in the algorithm
        // it does not return 0 never here.
        return sign_orientation_value(l, v, r) > 0 ? 1 : -1;
    }

    // tv
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
        int maximal_y = first->data.pt->y;

        for (unsigned int ix = 0; ix < list->size(); ++ix) {
            if(node->data.pt->y > maximal_y) {
                maximal_y = node->data.pt->y;
                maximal_index = node;
            } else if(node->data.pt->y == maximal_y) {
                if(node->data.pt->x < maximal_index->data.pt->x) {
                    maximal_y = node->data.pt->y;
                    maximal_index = node;
                }
            }

            node = node->successor();
        }

        return maximal_index;
    }

    bool EarClippingTriangulation::isDegenrate(const EarClippingTriangulation::Node *v) {
        const Node * l = v->predecessor();
        const Node * r = v->successor();

        bool test  = sign_orientation_value(l, v, r)==0;
        return test;
    }

    bool EarClippingTriangulation::isConvex(const EarClippingTriangulation::Node *v,
                                            const EarClippingTriangulation::LinkedList *list) {
        // the maximal y elemnt is always convex, therefore if
        // they have the same orientation, then v is also convex
        return neighborhood_orientation_sign(v) *
               neighborhood_orientation_sign(maximal_y_element(list)) > 0;
    }

    bool EarClippingTriangulation::areEqual(const EarClippingTriangulation::Node *a,
                                            const EarClippingTriangulation::Node *b) {
        return
            a->data.pt->x==b->data.pt->x &&
            a->data.pt->y==b->data.pt->y;
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

            if(areEqual(n, v) || areEqual(n, l) || areEqual(n, r))
                continue;
//            if(n==v || n==l || n==r)
//                continue;

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
            case triangles::TrianglesIndices::TRIANGLES_WITH_BOUNDARY:
                return (polygon_size - 2)*3;
            default:
                return 0;
        }

    }


}
