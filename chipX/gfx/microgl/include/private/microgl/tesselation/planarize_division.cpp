//#include "planarize_division.h"

#include "planarize_division.h"

namespace tessellation {

    // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
    // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))
    //    Input:  three points p, a, b
    //    Return: >0 for P2 left of the line through a and b
    //            =0 for P2  on the line
    //            <0 for P2  right of the line
    //    See: Algorithm 1 "Area of Triangles and Polygons"
    template <typename number>
    inline int
    planarize_division<number>::classify_point(const vertex & point, const vertex &a, const vertex & b)
    {
        auto result = ((b.x - a.x) * (point.y - a.y)
                       - (point.x - a.x) * (b.y - a.y) );

        if(result <0)
            return 1;
        else if(result > 0)
            return -1;
        else return 0;
    }

    template<typename number>
    auto planarize_division<number>::create_frame(const chunker<vertex> &pieces,
                                                  static_pool & pool) -> half_edge_face * {
        // find bbox of all
        const auto pieces_length = pieces.size();

        vertex left_top=pieces.raw_data()[0];
        vertex right_bottom=left_top;

        for (int ix = 0; ix < pieces_length; ++ix) {
            auto const piece = pieces[ix];
            const auto piece_size = piece.size;
            for (int jx = 0; jx < piece_size; ++jx) {
                const auto & current_vertex = piece.data[jx];
                if(current_vertex.x<left_top.x)
                    left_top.x = current_vertex.x;
                if(current_vertex.y<left_top.y)
                    left_top.y = current_vertex.y;
                if(current_vertex.x>right_bottom.x)
                    right_bottom.x = current_vertex.x;
                if(current_vertex.y>right_bottom.y)
                    right_bottom.y = current_vertex.y;

            }
        }

        left_top.x -= 10;
        left_top.y -= 10;
        right_bottom.x += 10;
        right_bottom.y += 10;

        auto * v0 = pool.get_vertex();
        auto * v1 = pool.get_vertex();
        auto * v2 = pool.get_vertex();
        auto * v3 = pool.get_vertex();

        v0->coords = left_top;
        v1->coords = {left_top.x, right_bottom.y};
        v2->coords = right_bottom;
        v3->coords = {right_bottom.x, left_top.y};

        // note, no need for twin edges
        auto * edge_0 = pool.get_edge();
        auto * edge_1 = pool.get_edge();
        auto * edge_2 = pool.get_edge();
        auto * edge_3 = pool.get_edge();

        // ccw from left-top vertex
        edge_0->origin = v0;
        edge_1->origin = v1;
        edge_2->origin = v2;
        edge_3->origin = v3;

        v0->edge = edge_0;
        v1->edge = edge_1;
        v2->edge = edge_2;
        v3->edge = edge_3;

        edge_0->next = edge_1;
        edge_1->next = edge_2;
        edge_2->next = edge_3;
        edge_3->next = edge_0;

        edge_0->prev = edge_3;
        edge_1->prev = edge_0;
        edge_2->prev = edge_1;
        edge_3->prev = edge_2;

        auto * face = pool.get_face();
        edge_0->face = face;
        edge_1->face = face;
        edge_2->face = face;
        edge_3->face = face;

        // CCW around face, face is always to the left of the walk
        face->edge = edge_0;

        return face;
        // note: we might find out that edge-face pointer and edge-prev are not required.
    }

    template<typename number>
    auto planarize_division<number>::build_edges_and_conflicts(const chunker<vertex> &pieces,
                                                               half_edge_face & main_frame,
                                                               static_pool & pool) -> half_edge ** {

        const auto pieces_length = pieces.size();
        conflict * conflict_first = nullptr;
        conflict * conflict_last = nullptr;
        const auto v_size = pieces.unchunked_size();
        auto ** edges_list = new half_edge*[v_size];
        index edges_list_counter = 0;

        // build edges and twins, do not make next/prev connections
        for (int ix = 0; ix < pieces_length; ++ix) {
            half_edge * edge_first = nullptr;
            half_edge * edge_last = nullptr;

            auto const piece = pieces[ix];
            const auto piece_size = piece.size;
            for (int jx = 0; jx < piece_size; ++jx) {
                // v, e, c
                auto * v = pool.get_vertex();
                auto * e = pool.get_edge();
                auto * c = pool.get_conflict_node();

                // hook current v to e and c
                v->coords = piece.data[jx];
                v->edge = e;
                e->origin = v;
                c->vertex = v;
                c->vertex->type = point_type::unknown;
                c->vertex->conflict_face = &main_frame;
                edges_list[edges_list_counter++] = e;

                // record first edge of polygon
                if(edge_first==nullptr) {
                    edge_first = e;
                }

                // set last twin
                if(edge_last) {
                    auto * e_last_twin = pool.get_edge();
                    e_last_twin->origin = v;
                    edge_last->twin = e_last_twin;
                    e_last_twin->twin = edge_last;
                }

                if(conflict_last) {
                    conflict_last->next = c;
                } else {
                    conflict_first = c;
                }

                edge_last = e;
                conflict_last = c;
            }

            // hook the last edge, from last vertex into first vertex
            // of the current polygon
            if(edge_last) {
                auto * e_last_twin = pool.get_edge();
                e_last_twin->origin = edge_first->origin;
                edge_last->twin = e_last_twin;
                e_last_twin->twin = edge_last;
            }

        }

        main_frame.conflict_list = conflict_first;

        return edges_list;
    }

    template<typename number>
    auto planarize_division<number>::infer_trapeze(const half_edge_face *face) -> trapeze {
        auto * e = face->edge;
        const auto * e_end = face->edge;
        trapeze trapeze;
        trapeze.left_top = trapeze.left_bottom = trapeze.right_bottom = trapeze.right_top = e;

        do {
            const auto * v = e->origin;
            auto curr_x = v->coords.x;
            auto curr_y = v->coords.y;

            if(curr_x < trapeze.left_top->origin->coords.x || (curr_x == trapeze.left_top->origin->coords.x && curr_y < trapeze.left_top->origin->coords.y))
                trapeze.left_top = e;
            if(curr_x < trapeze.left_bottom->origin->coords.x || (curr_x == trapeze.left_bottom->origin->coords.x && curr_y > trapeze.left_bottom->origin->coords.y))
                trapeze.left_bottom = e;
            if(curr_x > trapeze.right_bottom->origin->coords.x || (curr_x == trapeze.right_bottom->origin->coords.x && curr_y > trapeze.right_bottom->origin->coords.y))
                trapeze.right_bottom = e;
            if(curr_x > trapeze.right_top->origin->coords.x || (curr_x == trapeze.right_top->origin->coords.x && curr_y < trapeze.right_top->origin->coords.y))
                trapeze.right_top = e;

            e=e->next;
        } while(e!=e_end);

        return trapeze;
    }

    template<typename number>
    auto planarize_division<number>::classify_point_with_trapeze(const vertex & point, const trapeze &trapeze) -> point_with_trapeze {
        if(point==trapeze.left_top) return point_with_trapeze::boundaey_vertex;
        if(point==trapeze.left_bottom) return point_with_trapeze::boundaey_vertex;
        if(point==trapeze.right_bottom) return point_with_trapeze::boundaey_vertex;
        if(point==trapeze.right_top) return point_with_trapeze::boundaey_vertex;
        if(classify_point(point, trapeze.left_top->origin->coords, trapeze.left_bottom->origin->coords)==0) return point_with_trapeze::left_wall;
        if(classify_point(point, trapeze.right_top->origin->coords, trapeze.right_bottom->origin->coords)==0) return point_with_trapeze::right_wall;
        if(classify_point(point, trapeze.left_top->origin->coords, trapeze.right_top->origin->coords)==0) return point_with_trapeze::top_wall;
        if(classify_point(point, trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords)==0) return point_with_trapeze::bottom_wall;
        return point_with_trapeze::inside;
    }

    template<typename number>
    auto planarize_division<number>::split_edge_at(half_edge_vertex * point,
                                                   half_edge *edge, dynamic_pool & pool) -> half_edge * {
        // let's shorten both edge and it's twin,each from it's side
        // main frame does not have twins because are needed.
        //  --e0---*--e1--->
        // <--et1--*--et0--
        bool has_twin = edge->twin!=nullptr;
        auto * e_0 = edge;
        auto * e_1 = pool.create_edge();
        auto * e_t_0 = edge->twin;
        auto * e_t_1 = has_twin ? pool.create_edge() : nullptr;

        e_1->origin = point;
        e_1->prev = e_0;
        e_1->next = e_0->next;
        e_0->next->prev = e_1;

        e_0->next = e_1;
        // inherit face and winding properties
        e_1->face = e_0->face;
        e_1->winding = e_0->winding;

        if(has_twin) {
            e_t_1->origin = point;
            e_t_1->prev = e_t_0;
            e_t_1->next = e_t_0->next;
            e_t_0->next->prev = e_t_1;

            e_t_1->twin = e_0;
            e_0->twin = e_t_1;

            e_t_0->next = e_t_1;
            e_t_0->twin = e_1;
            e_1->twin = e_t_0;
            // inherit face and winding properties
            e_t_1->face = e_t_0->face;
            e_t_1->winding = e_t_0->winding;

        }

        // make sure point refers to any edge leaving it
        point->edge = e_1;

        return e_1;
    }

    template<typename number>
    auto planarize_division<number>::try_insert_vertex_on_trapeze_boundary_at(const vertex & v,
                                                                              const trapeze & trapeze,
                                                                              point_with_trapeze where_boundary,
                                                                              dynamic_pool & pool) -> half_edge * {
        // given where on the boundary: left, top, right, bottom
        // walk along that boundary ray and insert the vertex at the right place.
        // if the vertex already exists, do nothing ?
        // otherwise, insert a vertex and split the correct edge segment of the ray.
        // at the end, return the corresponding half-edge, whose origin is the vertex.
        // note:: it is better to return the edge than only the vertex, because
        // the edge locates the vertex, while the vertex locates any edge that leaves it,
        // and therefore would require traversing around the vertex to find the edge that belongs
        // to the face we are interested in.
        half_edge * e_start = nullptr;
        half_edge * e_end = nullptr;
        bool compare_y = false;
        bool reverse_direction = false;

        switch (where_boundary) {
            case point_with_trapeze::inside:
                return nullptr;
            case point_with_trapeze::boundary_vertex:
                if(v==trapeze.left_top) return trapeze.left_top;
                if(v==trapeze.left_bottom) return trapeze.left_bottom;
                if(v==trapeze.right_bottom) return trapeze.right_bottom;
                if(v==trapeze.right_top) return trapeze.right_top;
                break;
            case point_with_trapeze::left_wall:
                e_start = trapeze.left_top; e_end = trapeze.left_bottom->next;
                compare_y=true;reverse_direction=false;
                break;
            case point_with_trapeze::right_wall:
                e_start = trapeze.right_bottom; e_end = trapeze.right_top->next;
                compare_y=true;reverse_direction=true;
                break;
            case point_with_trapeze::top_wall:
                e_start = trapeze.right_top; e_end = trapeze.left_top->next;
                compare_y=false;reverse_direction=true;
                break;
            case point_with_trapeze::bottom_wall:
                e_start = trapeze.left_bottom; e_end = trapeze.right_bottom->next;
                compare_y=false;reverse_direction=false;
                break;

        }

        const auto * e_end_ref = e_end;

        do {
            auto coord_0 = compare_y ? e_start->origin->coords.y : e_start->origin->coords.x;
            auto coord_1 = compare_y ? e_start->next->origin->coords.y : e_start->next->origin->coords.x;
            auto v_coord = compare_y ? v.y : v.x;
            bool on_segment = reverse_direction ? v_coord>coord_1 : v_coord<coord_1;

            // v lies on e
            if(on_segment) {
                // check endpoints first
                if(v_coord==coord_0)
                    return e_start;
                auto *point = pool.create_vertex(v);
                return split_edge_at(point, e_start, pool);
            }
            e_start=e_start->next;
        } while(e_start!=e_end_ref);

    }

    template<typename number>
    number planarize_division<number>::evaluate_line_at_x(const number x, const vertex &a, const vertex &b) {
        auto slope = (b.y-a.y)/(b.x-a.x);
        auto y = a.y + slope*(x-a.x);
        return y;
    }

    template<typename number>
    void planarize_division<number>::remove_edge_from_conflict_list(half_edge * edge) {
        auto * conflict_face = edge->origin->conflict_face;
        conflict * list_ref = conflict_face->conflict_list;
        conflict * list_ref_prev = nullptr;
        const auto * vertex_ref = edge->origin;

        do {
            if(list_ref->vertex==vertex_ref) {

                if(list_ref_prev!=nullptr)
                    list_ref_prev->next = list_ref->next;
                else // in case, the first node was to be removed
                    conflict_face->conflict_list=list_ref->next;

                // reset the node
                list_ref->next=nullptr;

                break;
            }

            list_ref_prev = list_ref;
            list_ref=list_ref->next;
        } while(list_ref!= nullptr);

        // clear the ref from vertex to face
        edge->origin->conflict_face = nullptr;
    }

    template<typename number>
    auto planarize_division<number>::classify_conflict_against_two_faces(const half_edge* face_seperator, const half_edge* edge) -> half_edge_face *{
        // note:: edge's face always points to the face that lies to it's left.
        // 1. if the first point lie completely to the left of the edge, then they belong to f1, other wise f2
        // 2. if they lie exactly on the edge, then we test the second end-point
        // 2.1 if the second lies completely in f1, then the first vertex is in f1, otherwise f2
        // 2.2 if the second lies on the edge as well, choose whoever face you want, let's say f1
        const auto & a = face_seperator->origin->coords;
        const auto & b = face_seperator->twin->origin->coords;
        const auto & c = edge->origin->coords;
        const auto & d = edge->origin->coords;
        int cls = classify_point(a, c, d);

        if(cls>0) // if strictly left of
            return face_seperator->face;
        else if(cls<0)// if strictly right of
            return face_seperator->twin->face;
        else { // id exactly on separator
            int cls2 = classify_point(b, c, d);
            if(cls2>0)// if strictly left of
                return face_seperator->face;
            else if(cls2<0)// if strictly right of
                return face_seperator->twin->face;
        }
        // both lie on the separator
        return face_seperator->face;
    }

    template<typename number>
    void planarize_division<number>::insert_edge_between_non_co_linear_vertices(half_edge *vertex_a_edge, half_edge *vertex_b_edge,
                                                                                dynamic_pool & pool) {
        // insert edge between two vertices in a face, that are not co linear located by their leaving edges.
        // co linearity means the vertices lie on the same boundary ray. for that case, we have a different
        // procedure to handle.
        // first create two half edges
        auto * face = vertex_a_edge->face;
        auto * face_2 = pool.create_face();

        auto * e = pool.create_edge();
        auto * e_twin = pool.create_edge();
        // e is rooted at a, e_twin at b
        e->origin = vertex_a_edge->origin;
        e_twin->origin = vertex_b_edge->origin;
        // make them twins
        e->twin = e_twin;
        e_twin->twin = e;
        // rewire outgoing
        vertex_a_edge->prev->next = e;
        e->prev = vertex_a_edge->prev;
        vertex_b_edge->prev->next = e_twin;
        e_twin->prev = vertex_b_edge->prev;
        // rewire incoming
        e->next = vertex_b_edge;
        vertex_b_edge->prev = e;
        e_twin->next = vertex_a_edge;
        vertex_a_edge->prev = e_twin;

        // now, we need to update conflicts lists and link some edges tot heir new face
        // so, now face will be f1, the face left of e. while e-twin will have f2 as left face
        e->face = face;
        e_twin->face = face_2;
        // now, iterate on all of the conflicts of f, and move the correct ones to f2
        



    }

    template<typename number>
    auto planarize_division<number>::handle_start_face(half_edge_face *face, half_edge *edge,
                                                        dynamic_pool & dynamic_pool) -> split_result {
        // the mission of this method is to split a little to create an intermediate face
        // for the regular procedure, this procedure might split a face to two or just
        // split an edge on which the vertex lies on.
        // thw two end points of the edge
        const auto & a = edge->origin->coords;
        const auto & b = edge->twin->origin->coords;
        // find trapeze boundaries
        trapeze trapeze = infer_trapeze(face);
        // todo: what about degenerate trapeze with three vertices ?

        // so,
        // two things can occur, the vertex is completely inside the trapeze
        // or the vertex is on the boundary.
        // 1. if v is on boundary:
        // 1.1. v is on the vertical lines, in this case just split the boundary
        // 1.2. v is on the ceil or floor of the trapeze, in this case, split with a vertical line
        //      the above and below and split to two faces
        // 1.3. v is already existing, then just replace the reference to the one present already
        // 2. if v is completely inside, then split with a vertical edge into two faces

        // test point status
        point_with_trapeze classs = classify_point_with_trapeze(a, trapeze);
        // boundary
        bool on_boundary = classs!=point_with_trapeze::inside;
        bool on_boundary_vertices = classs==point_with_trapeze::boundaty_vertex;
        // completely inside minus the endpoints
        bool in_left_wall = classs==point_with_trapeze::left_wall;
        bool in_right_wall = classs==point_with_trapeze::right_wall;
        bool in_top_wall = classs==point_with_trapeze::top_wall;
        bool in_bottom_wall = classs==point_with_trapeze::bottom_wall;
        // should split the horizontal parts of the trapeze
        bool should_split_horizontal_trapeze_parts = !in_left_wall && !in_right_wall && !on_boundary_vertices;


        // should split the horizontal parts of the trapeze
        if(should_split_horizontal_trapeze_parts) {
            half_edge *top_vertex_edge= nullptr, *bottom_vertex_edge= nullptr;
            // we are on the top or bottom boundary, we should try insert a vertex there,
            // this helps with future errors
            if(on_boundary) {
                // split boundary and return the edge whose origin is the split vertex
                half_edge * e = try_insert_vertex_on_trapeze_boundary_at(a, trapeze, classs, dynamic_pool);
                if(in_top_wall) top_vertex_edge=e;
                else bottom_vertex_edge=e;
            }

            if(top_vertex_edge== nullptr) {
                auto y = evaluate_line_at_x(a.x, trapeze.left_top->origin->coords, trapeze.right_top->origin->coords);
                top_vertex_edge=try_insert_vertex_on_trapeze_boundary_at({a.x,y}, trapeze, point_with_trapeze::top_wall, dynamic_pool);
            }

            if(bottom_vertex_edge== nullptr) {
                auto y = evaluate_line_at_x(a.x, trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords);
                bottom_vertex_edge=try_insert_vertex_on_trapeze_boundary_at({a.x,y}, trapeze, point_with_trapeze::bottom_wall, dynamic_pool);
            }

            // now, we need to split face in two

        }


    }

    template<typename number>
    auto planarize_division<number>::split_intermediate_face_with_edge(half_edge_face *face, half_edge *edge,
                                                                       dynamic_pool & pool) -> half_edge * {

        // thw two end points of the edge
        const auto & a = edge->origin->coords;
        const auto & b = edge->twin->origin->coords;
        // find trapeze boundaries
        trapeze trapeze = infer_trapeze(face);

        // now we have restored trapeze boundaries, todo: what about degenerate trapeze with three vertices ?
        // now test intersections against boundaries



    }

    template<typename number>
    void planarize_division<number>::insert_edge(half_edge *edge, dynamic_pool & pool) {

        // record the conflicting face of the origin point
        auto * conflicting_face = edge->origin->conflict_face;
        // thw two end points of the edge
        const auto & a = edge->origin->coords;
        const auto & b = edge->twin->origin->coords;

        // at first, suppose no edge cases
        // notes:
        // 1. every edge splits a face to most 4 faces
        // 2. edge cases are when it just lies on an already present edge
        // 2.1. if it lies on an edge, then split the edge
        // 2.2. if it lies on a vertex, then insert edge at correct position like a clock

        // let's start with just splitting a face at up to 4 places
        handle_start_face(conflicting_face, edge, pool);
        split_intermediate_face_with_edge(conflicting_face, edge, pool);

    }

    template<typename number>
    void planarize_division<number>::compute(const chunker<vertex> &pieces) {

        // vertices size is also edges size since these are polygons
        const auto v_size = pieces.unchunked_size();
        // plus 4s for the sake of frame
        static_pool static_pool(v_size + 4, 4 + v_size*2, 1, v_size);
        dynamic_pool dynamic_pool{};
        // create the main frame
        auto * main_face = create_frame(pieces, static_pool);
        // create edges and conflict lists
        auto ** edges_list = build_edges_and_conflicts(pieces, *main_face, static_pool);
        // todo: here create a random permutation of edges

        // now start iterations
        for (int ix = 0; ix < v_size; ++ix) {
            auto * e = edges_list[ix];
            remove_edge_from_conflict_list(e);
            insert_edge(e, dynamic_pool);
        }

    }

    template<typename number>
    bool planarize_division<number>::segment_intersection_test(const vertex &a, const vertex &b,
                                                               const vertex &c, const vertex &d,
                                                               vertex & intersection,
                                                               float &alpha, float &alpha1) {
        // vectors
        auto ab = b - a;
        auto cd = d - c;
        auto dem = ab.x * cd.y - ab.y * cd.x;

        // parallel lines
        // todo:: revisit when thinking about fixed points
        if (abs(dem) < 0.0001f)
            return intersection::parallel;
        else {
            auto ca = a - c;
            auto ac = -ca;
            auto numerator = ca.y * cd.x - ca.x * cd.y;

            if (dem > 0) {
                if (numerator < 0 || numerator > dem)
                    return intersection::none;
            } else {
                if (numerator > 0 || numerator < dem)
                    return intersection::none;
            }

            alpha = numerator / dem;
//            alpha1 = (ab.y * ac.x - ab.x * ac.y) / dem;
            intersection = a + ab * alpha;
        }

        return intersection::intersect;
    }


    /*
    template <typename number>
    void planarize_division<number>::find_intersections(chunker<vertex> &pieces,
                                                        master_intersection_list &master_list,
                                                        dynamic_array<vertex *> &allocated_intersection) {
        // phase 1:: record trivial intersections
        for (unsigned long poly = 0; poly < pieces.size(); ++poly) {
            auto current_chunk = pieces[poly];
            auto *current_list = current_chunk.data;
            const auto size = current_chunk.size;

            for (unsigned long ix = 0; ix < size; ++ix) {
                int ix_next = ix + 1 >= size ? 0 : int(ix + 1);
                int ix_prev = int(ix - 1) < 0 ? int(size - 1) : int(ix - 1);
                vertex *current = &current_list[ix];
                vertex *next = &current_list[ix_next];
                vertex *prev = &current_list[ix_prev];

                segment l1(prev, current);
                segment l2(current, next);

                l1.sortVertices();
                l2.sortVertices();

                master_list.push_back(intersection(current, 2.0, -1.0, l1, l2));
            }
        }

        // phase 2:: find self intersections of each polygon
        for (unsigned long poly = 0; poly < pieces.size(); ++poly) {
            auto current_chunk = pieces[poly];
            auto *current_list = current_chunk.data;
            const auto size = current_chunk.size;

            if (size == 0)
                continue;

            for (unsigned long ix = 0; ix < size - 1; ++ix) {

                segment edge_0{&current_list[ix], &current_list[(ix + 1)]};

                for (unsigned long jx = ix + 1; jx < size; ++jx) {
                    segment edge_1{&current_list[jx],
                                   &current_list[(jx + 1) % size]};

                    vertex intersection_v;
                    float al1, al2;

                    // see if any of the segments have a mutual endpoint
                    if (!edge_1.is_bbox_overlapping_with(edge_0))
                        continue;
                    if (edge_1.has_mutual_endpoint(edge_0))
                        continue;
                    if (edge_1.calcIntersection(edge_0, intersection_v, al1, al2)
                        != segment::IntersectionType::INTERSECT)
                        continue;

                    auto *found_intersection = new vertex(intersection_v);

                    allocated_intersection.push_back(found_intersection);

                    master_list.push_back(intersection(found_intersection,
                                                       al1, al2, edge_1, edge_0));

                }
            }

        }

        // finds the intersection points between every polygon edge

        dynamic_array<segment> edges, edges1;

        for (unsigned long poly1 = 0; poly1 < pieces.size() - 1; ++poly1) {
            auto current_chunk1 = pieces[poly1];
            auto *current_poly1 = current_chunk1.data;
            const unsigned long size = current_chunk1.size;

            edges.clear();
            for (unsigned ix = 0; ix < size; ++ix) {
                unsigned ix_next = ix + 1 >= size ? 0 : ix + 1;
                vertex *current = &current_poly1[ix];
                vertex *next = &current_poly1[ix_next];

                segment edge(current, next);
                edge.sortVertices();
                edges.push_back(edge);
            }

            // sorting is crucial for the integrity of the output,
            // since I am applying an optimization to discard future edges based
            // on sorting
            qsort_s(edges.data(), edges.size(), sizeof(segment),
                    compare_edges, nullptr);

            for (unsigned long poly2 = poly1 + 1; poly2 < pieces.size(); ++poly2) {

                edges1.clear();
                auto current_chunk2 = pieces[poly2];
                auto *current_poly2 = current_chunk2.data;
                const unsigned long size2 = current_chunk2.size;

                for (unsigned ix = 0; ix < size2; ++ix) {
                    unsigned ix_next = ix + 1 >= size2 ? 0 : ix + 1;
                    vertex *current = &current_poly2[ix];
                    vertex *next = &current_poly2[ix_next];

                    segment edge(current, next);
                    edge.sortVertices();
                    edges1.push_back(edge);
                }

                qsort_s(edges1.data(), edges1.size(), sizeof(segment),
                        compare_edges, nullptr);

                unsigned nEdges = edges.size();
                unsigned nEdges1 = edges1.size();

                if (nEdges < 3)
                    continue;

                for (unsigned i = 0; i < nEdges; ++i) {
                    auto &edge_0 = edges[i];

                    for (unsigned j = 0; j < nEdges1; ++j) {

                        auto &edge_1 = edges1[j];

                        // if edge_1 is completely to the right of edge_0, then no intersection
                        // occurs, and also, since edge_1 syblings are sorted on the x-axis,
                        // we can skip it's upcoming syblings, therefore we break;
                        auto h_classify = edge_1.classify_horizontal(edge_0);
                        if (h_classify == segment::bbox_axis::end_of)
                            break;
                        else if (h_classify != segment::bbox_axis::overlaps)
                            continue;
                        else {
                            // we have horizontal overlap, let's test for vertical overlap
                            if (edge_1.classify_vertical(edge_0) != segment::bbox_axis::overlaps)
                                continue;
                        }

                        // see if any of the segments have a mutual endpoint
                        if (edge_1.has_mutual_endpoint(edge_0))
                            continue;

                        vertex intersection_v;
                        float param1, param2;

                        // test and compute intersection
                        if (edge_0.calcIntersection(edge_1, intersection_v, param1, param2)
                            != segment::IntersectionType::INTERSECT)
                            continue;

                        auto *found_intersection = new vertex(intersection_v);

                        allocated_intersection.push_back(found_intersection);

                        master_list.push_back(intersection(found_intersection,
                                                           param1, param2,
                                                           edge_0, edge_1));

                    }

                }

            }

        }
    }
*/
}