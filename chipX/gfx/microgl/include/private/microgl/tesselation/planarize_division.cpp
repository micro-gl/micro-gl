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
                e->type = edge_type::unknown;
                // only the first half edge records conflict info, the twin will not record !!
                e->conflict_face = &main_frame;
                c->edge = e;
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
    auto planarize_division<number>::classify_point_conflicting_trapeze(vertex & point, const trapeze &trapeze) -> point_class_with_trapeze {
        // given that point is in trapeze: completely inside or on boundary
        // we assume that the vertex DOES belong to the trapeze even if it may
        // be outside. WE ASSUME, that if it is outside or what not, then it is because
        // of numeric precision errors, that were carried over because vertical splits.
        // This can happen only for y coordinates because x coords are very precise. In this case,
        // we clamp the coord, if it flows the left/right walls y coords. As for inaccurate y coords
        // strictly inside the bottom/top walls, we report the point is inside without fixing
        // it's position, because other procedures can handle this case gracefully (as far as
        // I am aware of it).
        if(point==trapeze.left_top) return point_class_with_trapeze::boundary_vertex;
        if(point==trapeze.left_bottom) return point_class_with_trapeze::boundary_vertex;
        if(point==trapeze.right_bottom) return point_class_with_trapeze::boundary_vertex;
        if(point==trapeze.right_top) return point_class_with_trapeze::boundary_vertex;

        // left wall
        bool on_left_wall = point.x==trapeze.left_top->origin->coords.x;
        if(on_left_wall) {
            clamp(point.y, trapeze.left_top->origin->coords.y, trapeze.left_bottom->origin->coords.y);
            if(point==trapeze.left_top) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.left_bottom) return point_class_with_trapeze::boundary_vertex;
            return point_class_with_trapeze::left_wall;
        }
        // right wall
        bool on_right_wall = point.x==trapeze.right_top->origin->coords.x;
        if(on_right_wall) {
            clamp(point.y, trapeze.right_top->origin->coords.y, trapeze.right_bottom->origin->coords.y);
            if(point==trapeze.right_top) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.right_bottom) return point_class_with_trapeze::boundary_vertex;
            return point_class_with_trapeze::right_wall;
        }
        // bottom wall
        int bottom_wall = classify_point(point, trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords);
        if(bottom_wall==0) return point_class_with_trapeze::bottom_wall;
        // top wall
        int top_wall = classify_point(point, trapeze.right_top->origin->coords, trapeze.left_top->origin->coords);
        if(top_wall==0) return point_class_with_trapeze::top_wall;

        return point_class_with_trapeze::strictly_inside;
    }

    template<typename number>
    auto planarize_division<number>::classify_arbitrary_point_with_trapeze(vertex & point, const trapeze &trapeze) -> point_class_with_trapeze {
        // given any point, classify it against the trapeze, this is robust
        if(point==trapeze.left_top) return point_class_with_trapeze::boundary_vertex;
        if(point==trapeze.left_bottom) return point_class_with_trapeze::boundary_vertex;
        if(point==trapeze.right_bottom) return point_class_with_trapeze::boundary_vertex;
        if(point==trapeze.right_top) return point_class_with_trapeze::boundary_vertex;

        // left wall
        number left_wall = point.x-trapeze.left_top->origin->coords.x;
        number right_wall = -point.x+trapeze.left_top->origin->coords.x;
        int bottom_wall = classify_point(point, trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords);
        int top_wall = classify_point(point, trapeze.right_top->origin->coords, trapeze.left_top->origin->coords);

        bool inside_or_boundary = left_wall>=0 && right_wall>=0 && bottom_wall>=0 && top_wall>=0;
        if(inside_or_boundary) {
            if(left_wall==0) return point_class_with_trapeze::left_wall;
            if(right_wall==0) return point_class_with_trapeze::right_wall;
            if(bottom_wall==0) return point_class_with_trapeze::bottom_wall;
            if(top_wall==0) return point_class_with_trapeze::top_wall;

            return point_class_with_trapeze::strictly_inside;
        }
        return point_class_with_trapeze::outside;
    }

    // note: two vertices on the same wall are symbolically on a straight line if if they have numeric inaccuracies

    template<typename number>
    auto planarize_division<number>::try_split_edge_at(const vertex& point,
                                                       half_edge *edge, dynamic_pool & pool) -> half_edge * {
        // let's shorten both edge and it's twin,each from it's side
        // main frame does not have twins because are needed.
        //  --e0---*--e1--->
        // <--et1--*--et0--
        // if the point is already one of the edges endpoints skip fast
        if(point==edge->origin->coords)
            return edge;
        if(point==edge->twin->origin->coords)
            return edge->twin;

        bool has_twin = edge->twin!=nullptr;
        auto * e_0 = edge;
        auto * e_1 = pool.create_edge();
        auto * e_t_0 = edge->twin;
        auto * e_t_1 = has_twin ? pool.create_edge() : nullptr;

        e_1->origin = pool.create_vertex(point);
        e_1->prev = e_0;
        e_1->next = e_0->next;
        e_0->next->prev = e_1;

        e_0->next = e_1;
        // inherit face and winding properties
        e_1->face = e_0->face;
        e_1->winding = e_0->winding;

        if(has_twin) {
            e_t_1->origin = e_1->origin;
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
        e_1->origin->edge = e_1;

        return e_1;
    }

    template<typename number>
    auto planarize_division<number>::try_insert_vertex_on_trapeze_boundary_at(const vertex & v,
                                                                              const trapeze & trapeze,
                                                                              point_class_with_trapeze where_boundary,
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
            case point_class_with_trapeze::strictly_inside:
                return nullptr;
            case point_class_with_trapeze::boundary_vertex:
                if(v==*trapeze.left_top) return trapeze.left_top;
                if(v==*trapeze.left_bottom) return trapeze.left_bottom;
                if(v==*trapeze.right_bottom) return trapeze.right_bottom;
                if(v==*trapeze.right_top) return trapeze.right_top;
                break;
            case point_class_with_trapeze::left_wall:
                e_start = trapeze.left_top; e_end = trapeze.left_bottom->next;
                compare_y=true;reverse_direction=false;
                break;
            case point_class_with_trapeze::right_wall:
                e_start = trapeze.right_bottom; e_end = trapeze.right_top->next;
                compare_y=true;reverse_direction=true;
                break;
            case point_class_with_trapeze::top_wall:
                e_start = trapeze.right_top; e_end = trapeze.left_top->next;
                compare_y=false;reverse_direction=true;
                break;
            case point_class_with_trapeze::bottom_wall:
                e_start = trapeze.left_bottom; e_end = trapeze.right_bottom->next;
                compare_y=false;reverse_direction=false;
                break;
            default:
                return nullptr;

        }

        const auto * e_end_ref = e_end;

        do {
            auto coord_0 = compare_y ? e_start->origin->coords.y : e_start->origin->coords.x;
            auto coord_1 = compare_y ? e_start->next->origin->coords.y : e_start->next->origin->coords.x;
            auto v_coord = compare_y ? v.y : v.x;
            bool on_segment = reverse_direction ? v_coord>coord_1 : v_coord<coord_1;

            // there can only be one vertex on the principal axis with the same 1D coord(x or y)
            // v lies on e
            if(on_segment) {
                // check endpoints first
                if(v_coord==coord_0)
                    return e_start;
                auto *point = pool.create_vertex(v);
                return try_split_edge_at(point, e_start, pool);
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

    /*
    template<typename number>
    void planarize_division<number>::remove_edge_from_conflict_list(half_edge * edge) {
        auto * conflict_face = edge->origin->conflict_face;
        conflict * list_ref = conflict_face->conflict_list;
        conflict * list_ref_prev = nullptr;
        const auto * edge_ref = edge;

        while(list_ref!= nullptr) {
            if(list_ref->edge==edge_ref) {

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
        }

        // clear the ref from vertex to face
        edge->origin->conflict_face = nullptr;
    }
     */

    template<typename number>
    void planarize_division<number>::re_distribute_conflicts_of_split_face(conflict *conflict_list,
                                                                           const half_edge* face_separator) {
        // given that a face f was just split into two faces with
        // face_separator edge, let's redistribute the conflicts

        auto * f1 = face_separator->face;
        auto * f2 = face_separator->twin->face;

        f1->conflict_list = nullptr;
        f2->conflict_list = nullptr;

        conflict * list_ref = conflict_list;

        while(list_ref!= nullptr) {
            auto * current_ref = list_ref;
            // de-attach and advance
            current_ref->next = nullptr;
            auto * e = current_ref->edge;
            // find the face to which the edge is classified
            const auto * f = classify_conflict_against_two_faces(face_separator, e);
            const auto * f_conflict_list=f->conflict_list;
            // insert edge into correct conflict list
            if(f_conflict_list)
                f_conflict_list->next=current_ref;
            else
                f->conflict_list = current_ref;
            // pointer from edge to conflicting face
            e->conflict_face=f;
            list_ref=list_ref->next; // advance
        }

    }

    template<typename number>
    auto planarize_division<number>::classify_conflict_against_two_faces(const half_edge* face_separator, const half_edge* edge) -> half_edge_face *{
        // note:: edge's face always points to the face that lies to it's left.
        // 1. if the first point lie completely to the left of the edge, then they belong to f1, other wise f2
        // 2. if they lie exactly on the edge, then we test the second end-point
        // 2.1 if the second lies completely in f1, then the first vertex is in f1, otherwise f2
        // 2.2 if the second lies on the face_separator as well, choose whoever face you want, let's say f1
        const auto & a = face_separator->origin->coords;
        const auto & b = face_separator->twin->origin->coords;
        const auto & c = edge->origin->coords;
        const auto & d = edge->origin->coords;
        int cls = classify_point(a, c, d);

        if(cls>0) // if strictly left of
            return face_separator->face;
        else if(cls<0)// if strictly right of
            return face_separator->twin->face;
        else { // id exactly on separator
            int cls2 = classify_point(b, c, d);
            if(cls2>0)// if strictly left of
                return face_separator->face;
            else if(cls2<0)// if strictly right of
                return face_separator->twin->face;
        }
        // both lie on the separator
        return face_separator->face;
    }

    template<typename number>
    void planarize_division<number>::walk_and_update_edges_face(half_edge * edge_start, const half_edge_face * face) {
        // If I only use this once, then embed it inside it's only consumer
        auto * e_ref = edge_start;
        const auto * e_end = edge_start;
        do {
            e_ref->face = face;
            e_ref = e_ref->next;
        } while(e_ref!=nullptr && e_ref!=e_end);
    }

    template<typename number>
    auto planarize_division<number>::insert_edge_between_non_co_linear_vertices(half_edge *vertex_a_edge,
                                                                                half_edge *vertex_b_edge,
                                                                                dynamic_pool & pool) -> half_edge * {
        // insert edge between two vertices in a face, that are not co linear located
        // by their leaving edges. co linearity means the vertices lie on the same boundary ray.
        // for that case, we have a different procedure to handle.
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

        // now, we need to update conflicts lists and link some edges to their new faces
        // so, now face will be f1, the face left of e. while e-twin will have f2 as left face
        e->face = face;
        e_twin->face = face_2;
        // now, iterate on all of the conflicts of f, and move the correct ones to f2
        re_distribute_conflicts_of_split_face(e->face->conflict_list, e);
        // all edges of face_2 point to the old face_1, we need to change that
        walk_and_update_edges_face(e_twin, face_2);
        return e;
    }

    template<typename number>
    auto planarize_division<number>::handle_vertical_face_cut(const trapeze &trapeze,
                                                              vertex & a,
                                                              const point_class_with_trapeze &a_classs,
//                                                              half_edge_vertex * a,
                                                              dynamic_pool & dynamic_pool) -> half_edge * {
        // the procedure returns the half edge of the face for which it's origin vertex
        // is the insertion point. the half_edge_vertex *a will be inserted in place if it didn't exist yet.
        // the mission of this method is to split a little to create an intermediate face
        // for the regular procedure, this procedure might split a face to two or just
        // split an edge on which the vertex lies on.
        // todo: what about degenerate trapeze with three vertices ?
        // IMPORTANT NOTE:
        // suppose we have an edge E that is conflicting with a trapeze, and we split the trapeze
        // vertically into two trapezes because of another edge, also suppose that E was sitting on
        // the boundary or very close to the boundary of the top and bottom rays. after the split,
        // because of numeric precision, we should expect, that the classification of the edge should be
        // wrong against one of those rays. we need to deal with this:
        // 1. if E was on the left/right boundary exactly on the endpoints, then we need to clamp it
        // 2. if E was strictly inside the bottom/top rays, we can proceed normally with the x split,
        //    BUT, when we create the vertical edge, we need to clamp E against that edge !!!

        // so,
        // two things can occur, the vertex is completely inside the trapeze
        // 1. the vertex is strictly inside the horizontal part of the trapeze,
        //    therefore, add a vertical segment that cuts the top/bottom segments
        // 1.1. this also induces a splitting of the face into two faces including updating
        //      references and conflicts
        // 1.2. if the vertex was completely inside the trapeze(not on the top/bottom boundary),
        //      then split that vertical line as well. Note, that the vertical line is unique because
        //      of the strict horizontal inclusion.
        // 2. the vertex is on the left/right walls (including endpoints, i.e not necessarily strict inside),
        //    in this case just split the vertical left or right walls vertically.
        // 3. btw, in case, a vertex already exists, we do not split anything

        // boundary
        bool on_boundary = a_classs != point_class_with_trapeze::strictly_inside;
        bool on_boundary_vertices = a_classs == point_class_with_trapeze::boundaty_vertex;
        // completely inside minus the endpoints
        bool in_left_wall = a_classs == point_class_with_trapeze::left_wall;
        bool in_right_wall = a_classs == point_class_with_trapeze::right_wall;
        bool in_top_wall = a_classs == point_class_with_trapeze::top_wall;
        bool in_bottom_wall = a_classs == point_class_with_trapeze::bottom_wall;
        bool strictly_in_left_or_right_walls = (in_left_wall || in_right_wall) && !on_boundary_vertices;
        // should split the horizontal parts of the trapeze
        bool should_try_split_horizontal_trapeze_parts = !in_left_wall && !in_right_wall && !on_boundary_vertices;

        // vertical edge that a sits on
        half_edge * outgoing_vertex_edge=nullptr;
        // should split the horizontal parts of the trapeze
        if(should_try_split_horizontal_trapeze_parts) {
            half_edge *top_vertex_edge= nullptr, *bottom_vertex_edge= nullptr;
            // we are on the top or bottom boundary, we should try insert a vertex there,
            // this helps with future errors like geometric rounding, because the point is
            // already present there.
            if(on_boundary) {
                // split boundary and return the edge whose origin is the split vertex
                half_edge * e = try_insert_vertex_on_trapeze_boundary_at(a, trapeze, a_classs,
                        dynamic_pool);
                if(in_top_wall) top_vertex_edge=e;
                else bottom_vertex_edge=e;
                outgoing_vertex_edge = e;
            }

            if(top_vertex_edge== nullptr) {
                auto y = evaluate_line_at_x(a.x, trapeze.left_top->origin->coords,
                        trapeze.right_top->origin->coords);
                top_vertex_edge=try_insert_vertex_on_trapeze_boundary_at({a.x,y}, trapeze,
                                                                         point_class_with_trapeze::top_wall,
                                                                         dynamic_pool);
            }

            if(bottom_vertex_edge== nullptr) {
                auto y = evaluate_line_at_x(a.x, trapeze.left_bottom->origin->coords,
                        trapeze.right_bottom->origin->coords);
                bottom_vertex_edge=try_insert_vertex_on_trapeze_boundary_at({a.x,y}, trapeze,
                                                                            point_class_with_trapeze::bottom_wall,
                                                                            dynamic_pool);
            }

            // now, we need to split face in two
            // edge cannot exist yet because we are strictly inside horizontal part
            auto *start_vertical_wall = insert_edge_between_non_co_linear_vertices(bottom_vertex_edge,
                    top_vertex_edge,dynamic_pool);
            // clamp vertex to this new edge endpoints if it is before or after
            // this fights the geometric numeric precision errors, that can happen in y coords
            clamp_vertex(a, start_vertical_wall->origin->coords,
                    start_vertical_wall->twin->origin->coords);
            // if the vertex is on the edge boundary, it will not split of course
            outgoing_vertex_edge = try_split_edge_at(a, start_vertical_wall, dynamic_pool);
            // todo: here we can update the trapeze dimensions info fast if needed
        } // else we are on left or right walls already
        else {
            // we are on left or right boundary
            outgoing_vertex_edge=try_insert_vertex_on_trapeze_boundary_at(a, trapeze,
                                                                          a_classs, dynamic_pool);
        }

        return outgoing_vertex_edge;
    }

    template<typename number>
    auto planarize_division<number>::handle_intermediate_face_with_edge(half_edge_face *face, half_edge *edge,
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
    void planarize_division<number>::insert_edge(half_edge *edge, dynamic_pool & dynamic_pool) {

        // record the conflicting face of the origin point
        // thw two end points of the edge
        auto * a = edge->origin->coords;
        auto * b = edge->twin->origin->coords;
        auto * b_tag = b;
        const auto * conflict_face = edge->conflict_face;
        half_edge_face * face = conflict_face;
        // first face handling
        trapeze trapeze;

        while(face) {
            // represent the face as trapeze for efficient info
            trapeze = infer_trapeze(face);
            // the class of endpoint "a"
            point_class_with_trapeze class_a = classify_point_with_trapeze(*a, trapeze);
            // the reporting of and class of the next interesting point of the edge against trapeze
            conflicting_edge_intersection_status edge_status =
                    compute_conflicting_edge_intersection_against_trapeze(trapeze, *a, *b);
            b_tag = &edge_status.point_of_interest;
            point_class_with_trapeze class_b_tag = classify_point_with_trapeze(*b_tag, trapeze);

            bool co_linear_with_boundary = do_a_b_lies_on_same_trapeze_wall(trapeze, *a, *b_tag,
                    class_a, class_b_tag);
            // co-linear, so let's just insert vertices on the boundary
            if(co_linear_with_boundary) {

            }
            // here update face and new "a" pointer and iterate

        }


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

            //remove_edge_from_conflict_list(e);
            insert_edge(e, dynamic_pool);
        }

    }

    template<typename number>
    void planarize_division<number>::clamp(number &val, number & a, number &b) {
        if(val<a) val = a;
        if(val>b) val = b;
    }

    template<typename number>
    void planarize_division<number>::clamp_vertex(vertex &v, vertex & a, vertex &b) {
        // clamp a vertex to endpoint if it has over/under flowed it
        bool is_a_before_b = a.x<b.x || (a.x==b.x && a.y<b.y);
        // sort, so a is before b
        if(!is_a_before_b) {
            vertex c=a;a=b;b=c;
        }
        bool is_v_before_a = v.x<a.x || (v.x==a.x && v.y<a.y);
        if(is_v_before_a) v=a;
        bool is_v_after_b = v.x>b.x || (v.x==b.x && v.y>b.y);
        if(is_v_after_b) v=b;
    }

    template<typename number>
    bool planarize_division<number>::is_trapeze_degenerate(const trapeze & trapeze) {
        return (trapeze.left_top->origin==trapeze.left_bottom->origin) ||
                (trapeze.right_top->origin==trapeze.right_bottom->origin);
    }

    template<typename number>
    bool planarize_division<number>::do_a_b_lies_on_same_trapeze_wall(const trapeze & trapeze,
                                                                        const vertex &a,
                                                                        const vertex &b,
                                                                        const point_class_with_trapeze & a_class,
                                                                        const point_class_with_trapeze & b_class) {
        // given an edge (a,b) where both a and b belong to trapeze, test if the edge is co-linear with one of the
        // boundaries of the trapeze. we classify symbolically rather than analytically in order to be robust.
        // this procedure also handles degenerate trapezes.
        // skip inside
        if(a_class==point_class_with_trapeze::strictly_inside ||
            b_class==point_class_with_trapeze::strictly_inside)
            return false;

        bool a_on_boundary_vertex = a_class==point_class_with_trapeze::boundary_vertex;
        bool b_on_boundary_vertex = b_class==point_class_with_trapeze::boundary_vertex;
        bool same_class = a_class==b_class;

        // test if on same wall
        if(same_class && !a_on_boundary_vertex && !b_on_boundary_vertex)
            return true;
        // todo: what happens when trapeze is degenerate - a triangle
        bool a_on_wall, b_on_wall;
        // test left wall
        a_on_wall = a_class==point_class_with_trapeze::left_wall ||
                              (a_on_boundary_vertex &&
                               (a==trapeze.left_top->origin->coords || a==trapeze.left_bottom->origin->coords));
        b_on_wall = b_class==point_class_with_trapeze::left_wall ||
                              (b_on_boundary_vertex &&
                               (b==trapeze.left_top->origin->coords || b==trapeze.left_bottom->origin->coords));
        if(a_on_wall && b_on_wall)
            return true;

        // test right wall
        a_on_wall = a_class==point_class_with_trapeze::right_wall ||
                               (a_on_boundary_vertex &&
                                (a==trapeze.right_top->origin->coords || a==trapeze.right_bottom->origin->coords));
        b_on_wall = b_class==point_class_with_trapeze::right_wall ||
                               (b_on_boundary_vertex &&
                                (b==trapeze.right_top->origin->coords || b==trapeze.right_bottom->origin->coords));
        if(a_on_wall && b_on_wall)
            return true;

        // test top wall
        a_on_wall = a_class==point_class_with_trapeze::top_wall ||
                    (a_on_boundary_vertex &&
                     (a==trapeze.right_top->origin->coords || a==trapeze.left_top->origin->coords));
        b_on_wall = b_class==point_class_with_trapeze::top_wall ||
                    (b_on_boundary_vertex &&
                     (b==trapeze.right_top->origin->coords || b==trapeze.left_top->origin->coords));
        if(a_on_wall && b_on_wall)
            return true;

        // test bottom wall
        a_on_wall = a_class==point_class_with_trapeze::bottom_wall ||
                    (a_on_boundary_vertex &&
                     (a==trapeze.left_bottom->origin->coords || a==trapeze.right_bottom->origin->coords));
        b_on_wall = b_class==point_class_with_trapeze::bottom_wall ||
                    (b_on_boundary_vertex &&
                     (b==trapeze.left_bottom->origin->coords || b==trapeze.right_bottom->origin->coords));
        if(a_on_wall && b_on_wall)
            return true;

        return false;
    }

    template<typename number>
    auto planarize_division<number>::compute_conflicting_edge_intersection_against_trapeze(const trapeze & trapeze,
                                                                                           vertex &a,
                                                                                           const vertex &b) -> conflicting_edge_intersection_status {
        // given that edge (a,b), a is conflicting, i.e on boundary or completely inside
        // and we know that the edge passes through the trapeze or lies on the boundary,
        // find the second interesting point, intersection or overlap or completely inside
        // NOTES:
        // 1. we want to be robust
        // 2. although endpoint "a" belongs to the trapeze, there is a chance it might be outside geometrically,
        //    because of numeric precision errors, that occur during vertical splits
        // 3. therefore, we might get two proper intersections for (a,b) edge against
        //    top/bottom/left/right boundaries.
        // 4. therefore, always consider the intersection with the biggest alpha as what we want.
        // to make it robust and clear I propose:
        // 1. at first figure out if "b" is strictly inside or the boundary of the trapeze
        // 1.1. if it is return the point as is with the classification, we are done
        // 1.2 if it is outside, the we need to hunt intersections with the biggest alpha or so

        // this can be injected from outside
//        point_class_with_trapeze  cla_a = classify_point_conflicting_trapeze(a, trapeze);
        point_class_with_trapeze  cla_b = classify_arbitrary_point_with_trapeze(b, trapeze);
        conflicting_edge_intersection_status result{};
        // if it is inside or on the boundary, we are done
        if(cla_b!=point_class_with_trapeze::outside) {
            result.point_of_interest = b;
            result.class_of_interest_point = cla_b;
            return result;
        }

        // we now know, that b is outside, therefore hunt proper intersections
        vertex intersection_point{};
        number alpha, alpha1;
        number biggest_alpha=number(0);
        intersection_status status, biggest_status;
        bool is_bigger_alpha;

        // left-wall
        status = segment_intersection_test(a, b,
                trapeze.left_top->origin->coords, trapeze.left_bottom->origin->coords,
                intersection_point, alpha, alpha1);
        if(status==intersection_status::intersect) {
            is_bigger_alpha = alpha>=biggest_alpha;

            if(is_bigger_alpha) {
                biggest_alpha=alpha;
                result.point_of_interest = intersection_point;
                result.class_of_interest_point = point_class_with_trapeze::left_wall;
                // important to clamp for symbolic reasons as well in case of numeric errors.
                result.point_of_interest.x=trapeze.left_top->origin->coords.x;
                clamp(result.point_of_interest.y, trapeze.left_top.y, trapeze.left_bottom.y);
                if(result.point_of_interest==trapeze.left_top->origin->coords ||
                    result.point_of_interest==trapeze.left_bottom->origin->coords)
                    result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
            }
        }

        // right wall
        status = segment_intersection_test(a, b,
                                           trapeze.right_bottom->origin->coords, trapeze.right_top->origin->coords,
                                           intersection_point, alpha, alpha1);
        if(status==intersection_status::intersect) {
            is_bigger_alpha = alpha>=biggest_alpha;

            if(is_bigger_alpha) {
                biggest_alpha=alpha;
                result.point_of_interest = intersection_point;
                result.class_of_interest_point = point_class_with_trapeze::right_wall;
                // important to clamp for symbolic reasons as well in case of numeric errors.
                result.point_of_interest.x=trapeze.right_top->origin->coords.x;
                clamp(result.point_of_interest.y, trapeze.right_top.y, trapeze.right_bottom.y);
                if(result.point_of_interest==trapeze.right_top->origin->coords ||
                   result.point_of_interest==trapeze.right_bottom->origin->coords)
                    result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
            }
        }

        // bottom wall
        status = segment_intersection_test(a, b,
                                           trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords,
                                           intersection_point, alpha, alpha1);
        if(status==intersection_status::intersect) {
            is_bigger_alpha = alpha>=biggest_alpha;

            if(is_bigger_alpha) {
                biggest_alpha=alpha;
                result.point_of_interest = intersection_point;
                result.class_of_interest_point = point_class_with_trapeze::bottom_wall;
                // important to clamp for symbolic reasons as well in case of numeric errors.
                clamp_vertex(result.point_of_interest, trapeze.left_bottom->origin->coords,
                             trapeze.right_bottom->origin->coords);
                if(result.point_of_interest==trapeze.left_bottom->origin->coords ||
                   result.point_of_interest==trapeze.right_bottom->origin->coords)
                    result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
            }
        }

        // top wall
        status = segment_intersection_test(a, b,
                                           trapeze.right_top->origin->coords, trapeze.left_top->origin->coords,
                                           intersection_point, alpha, alpha1);
        if(status==intersection_status::intersect) {
            is_bigger_alpha = alpha>=biggest_alpha;

            if(is_bigger_alpha) {
                biggest_alpha=alpha;
                result.point_of_interest = intersection_point;
                result.class_of_interest_point = point_class_with_trapeze::top_wall;
                // important to clamp for symbolic reasons as well in case of numeric errors.
                clamp_vertex(result.point_of_interest, trapeze.right_top->origin->coords,
                             trapeze.left_top->origin->coords);
                if(result.point_of_interest==trapeze.right_top->origin->coords ||
                   result.point_of_interest==trapeze.left_top->origin->coords)
                    result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
            }
        }

        // now classify if (a,b) lies on the same wall

        return result;
    }


    template<typename number>
    auto planarize_division<number>::segment_intersection_test(const vertex &a, const vertex &b,
                                                               const vertex &c, const vertex &d,
                                                               vertex & intersection,
                                                               number &alpha, number &alpha1) -> intersection_status{
        // this procedure will find proper and improper(touches) intersections, but no
        // overlaps, since overlaps induce parallel classfication, this would have to
        // be resolved outside
        // vectors
        auto ab = b - a;
        auto cd = d - c;
        auto dem = ab.x * cd.y - ab.y * cd.x;

        // parallel lines
        // todo:: revisit when thinking about fixed points
//        if (abs(dem) <= 0.0001f)
        if (dem == 0)
            return intersection_status::parallel;
        else {
            auto ca = a - c;
            auto ac = -ca;
            auto numerator = ca.y * cd.x - ca.x * cd.y;

            if (dem > 0) {
                if (numerator < 0 || numerator > dem)
                    return intersection_status::none;
            } else {
                if (numerator > 0 || numerator < dem)
                    return intersection_status::none;
            }

            // a lies on c--d segment
            if(numerator==0) {
                alpha=0;
                intersection = a;
            } // b lies on c--d segment
            else if(numerator==dem) {
                alpha=1;
                intersection = b;
            }
            else { // proper intersection
                alpha = numerator / dem;
//            alpha1 = (ab.y * ac.x - ab.x * ac.y) / dem;
                intersection = a + ab * alpha;
            }
        }

        return intersection_status::intersect;
    }

}