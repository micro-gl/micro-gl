```c++
namespace microgl {

    namespace tessellation {

        template<typename number>
        auto planarize_division<number>::create_frame(const chunker<vertex> &pieces,
                                                      static_pool & pool) -> half_edge_face * {
            const auto pieces_length = pieces.size();
            vertex left_top=pieces.data()[0];
            vertex right_bottom=left_top;
            // find bbox of all
            for (index ix = 0; ix < pieces_length; ++ix) {
                auto const piece = pieces[ix];
                const auto piece_size = piece.size;
                for (index jx = 0; jx < piece_size; ++jx) {
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
            // extend bbox 10 points
            left_top.x -= number(10);
            left_top.y -= number(10);
            right_bottom.x += number(10);
            right_bottom.y += number(10);
            // vertices
            auto * v0 = pool.get_vertex();
            auto * v1 = pool.get_vertex();
            auto * v2 = pool.get_vertex();
            auto * v3 = pool.get_vertex();
            // coords
            v0->coords = left_top;
            v1->coords = {left_top.x, right_bottom.y};
            v2->coords = right_bottom;
            v3->coords = {right_bottom.x, left_top.y};
            // half edges of vertices
            auto * edge_0 = pool.get_edge();
            auto * edge_1 = pool.get_edge();
            auto * edge_2 = pool.get_edge();
            auto * edge_3 = pool.get_edge();
            // twins of edges
            auto * edge_0_twin = pool.get_edge();
            auto * edge_1_twin = pool.get_edge();
            auto * edge_2_twin = pool.get_edge();
            auto * edge_3_twin = pool.get_edge();
            // connect edges to vertices, CCW from_sampler left-top vertex
            edge_0->origin = v0; edge_0_twin->origin = v1;
            edge_1->origin = v1; edge_1_twin->origin = v2;
            edge_2->origin = v2; edge_2_twin->origin = v3;
            edge_3->origin = v3; edge_3_twin->origin = v0;
            v0->edge = edge_0;
            v1->edge = edge_1;
            v2->edge = edge_2;
            v3->edge = edge_3;
            // winding is zero for frame
            edge_0->winding = edge_0_twin->winding = 0;
            edge_1->winding = edge_1_twin->winding = 0;
            edge_2->winding = edge_2_twin->winding = 0;
            edge_3->winding = edge_3_twin->winding = 0;
            // connect edges among themselves
            edge_0->next = edge_1; edge_0_twin->next = edge_3_twin;
            edge_1->next = edge_2; edge_1_twin->next = edge_0_twin;
            edge_2->next = edge_3; edge_2_twin->next = edge_1_twin;
            edge_3->next = edge_0; edge_3_twin->next = edge_2_twin;
            edge_0->prev = edge_3; edge_0_twin->prev = edge_1_twin;
            edge_1->prev = edge_0; edge_1_twin->prev = edge_2_twin;
            edge_2->prev = edge_1; edge_2_twin->prev = edge_3_twin;
            edge_3->prev = edge_2; edge_3_twin->prev = edge_0_twin;
            // connect twins
            edge_0->twin = edge_0_twin; edge_0_twin->twin = edge_0;
            edge_1->twin = edge_1_twin; edge_1_twin->twin = edge_1;
            edge_2->twin = edge_2_twin; edge_2_twin->twin = edge_2;
            edge_3->twin = edge_3_twin; edge_3_twin->twin = edge_3;
            // create a face and connect it
            auto * face = pool.get_face();
            edge_0->face = face; edge_0_twin->face = nullptr; // nullptr for face is the outside world face marker
            edge_1->face = face; edge_1_twin->face = nullptr;
            edge_2->face = face; edge_2_twin->face = nullptr;
            edge_3->face = face; edge_3_twin->face = nullptr;
            // CCW around face, face is always to the left of the walk
            face->edge = edge_0;

            return face;
        }

        template<typename number>
        auto planarize_division<number>::build_edges_and_conflicts(const chunker<vertex> &pieces,
                                                                   half_edge_face & main_frame,
                                                                   static_pool & pool) -> half_edge ** {
            // given polygons, build their vertices, edges and conflicts
            const auto pieces_length = pieces.size();
            conflict * conflict_first = nullptr;
            const auto v_size = pieces.unchunked_size();
            auto ** edges_list = new half_edge*[v_size];
            index edges_list_counter = 0;

            // build edges and twins, do not make next/prev connections
            for (index ix = 0; ix < pieces_length; ++ix) {
                half_edge * edge_first = nullptr;
                half_edge * edge_last = nullptr;

                auto const piece = pieces[ix];
                const auto piece_size = piece.size;
                for (index jx = 0; jx < piece_size; ++jx) {
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

                    if(conflict_first)
                        c->next = conflict_first;

                    conflict_first = c;
                    edge_last = e;
                }

                // hook the last edge, from_sampler last vertex into first vertex
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
        auto planarize_division<number>::infer_trapeze(const half_edge_face *face) -> trapeze_t {
            if(face->edge==nullptr) {
#if DEBUG_PLANAR==true
                throw std::runtime_error("infer_trapeze()::trying to infer a trapeze of a probably merged/deleted face !!!");
#endif
                return {};
            }

            auto * e = face->edge;
            const auto * e_end = face->edge;
            trapeze_t trapeze;
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
        auto planarize_division<number>::round_vertex_to_trapeze(vertex & point,
                                                                 const trapeze_t &trapeze) -> point_class_with_trapeze {
            // given that point that should belong to trapeze, BUT may not sit properly or outside because
            // of precision errors, geometrically round it into place
            auto codes= compute_location_codes(point, trapeze);
            // boundary vertices
            if((codes.left_wall==0 && codes.top_wall==0) || (codes.left_wall==0 && codes.bottom_wall==0) ||
               (codes.right_wall==0 && codes.top_wall==0) || (codes.right_wall==0 && codes.bottom_wall==0))
                return point_class_with_trapeze::boundary_vertex;
            { // left wall
                const auto & start = trapeze.left_top->origin->coords;
                const auto & end = trapeze.left_bottom->origin->coords;
                if(codes.left_wall<=0) { // outside or on wall
                    point.x= start.x; // truncate x
                    clamp(point.y, start.y, end.y);
                    if(point==start || point==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::left_wall;
                }
            }
            { // right wall
                const auto & start = trapeze.right_bottom->origin->coords;
                const auto & end = trapeze.right_top->origin->coords;
                if(codes.right_wall<=0) {
                    point.x= start.x; // truncate x
                    clamp(point.y, end.y, start.y);
                    if(point==start || point==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::right_wall;
                }
            }
            { // bottom wall
                const auto & start = trapeze.left_bottom->origin->coords;
                const auto & end = trapeze.right_bottom->origin->coords;
                if(codes.bottom_wall<=0) {
                    point.y= evaluate_line_at_x(point.x, start, end);
                    return point_class_with_trapeze::bottom_wall;
                }
            }
            { // top wall
                const auto & start = trapeze.right_top->origin->coords;
                const auto & end = trapeze.left_top->origin->coords;
                if(codes.top_wall<=0) {
                    point.y= evaluate_line_at_x(point.x, end, start);
                    return point_class_with_trapeze::top_wall;
                }
            }

            return point_class_with_trapeze::strictly_inside;
        }

        template<typename number>
        auto planarize_division<number>::round_edge_to_trapeze(const vertex & a, vertex & b,
                                                               const point_class_with_trapeze & class_a,
                                                               const trapeze_t &trapeze)
        -> point_class_with_trapeze {
            // given a trapeze and an edge (a,b), that is supposed to conflict with it, i.e
            // cross it's interior or boundary, assume that due to precision errors, the edge
            // segment may fall outside, therefore geometrically round it INTO the trapeze if needed.
            // we assume, that (a) class is in the closure of the trapeze even if analytic evaluation
            // might show otherwise (due to numeric precision errors).
            // we have two cases:
            // 1. vertex (a) is on the boundary, therefore test if (b) overflows outside on the same
            //    boundary wall as (a). If so, clamp/round correct (b) to fit in trapeze.
            // 2. if vertex (a) is the interior of trapeze, do nothing but classifying vertex (b)
            // NOTE: in any case report vertex (b) class.

            // preliminary assumption checkup, that a is not outside classified
            if(class_a==point_class_with_trapeze::outside) {
#if DEBUG_PLANAR==true
                throw std::runtime_error("round_edge_to_trapeze():: (a) does not belong to interior !!!");
#endif
                return point_class_with_trapeze::unknown;
            }
            bool a_in_interior= class_a==point_class_with_trapeze::strictly_inside;
            bool a_on_boundary= !a_in_interior;

            // easy test if (b) is in the closure of trapeze
            const auto codes= compute_location_codes(b, trapeze);
            const auto class_b = classify_from_location_codes(codes);
            if(class_b!=point_class_with_trapeze::outside) // b in closure, we are done
                return class_b;

            // if we got here, (b) is outside, now let's round things
            // if (a) is in interior, we are done since there is nothing to round, because there is intersection.
            if(a_in_interior)
                return point_class_with_trapeze::outside;

            // this might be redundant
            bool a_is_boundary_vertex = class_a==point_class_with_trapeze::boundary_vertex;
            // vertex (a) is on boundary since it is not outside and not in interior, now begin edge rounding if possible.
            // for each wall, test if vertex (a) is on the wall and that vertex (b) is strictly right of the wall.
            // if so, clamp it along.

            { // left wall
                const auto & start= trapeze.left_top->origin->coords;
                const auto & end= trapeze.left_bottom->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::left_wall ||
                                       (a_is_boundary_vertex && a.x==start.x);
                const bool b_right_of_wall = codes.left_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    b.x= start.x;
                    clamp(b.y, start.y, end.y);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::left_wall;
                }
            }
            { // right wall
                const auto & start= trapeze.right_bottom->origin->coords;
                const auto & end= trapeze.right_top->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::right_wall ||
                                       (a_is_boundary_vertex && a.x==start.x);
                const bool b_right_of_wall = codes.right_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    b.x= start.x;
                    clamp(b.y, end.y, start.y);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::right_wall;
                }
            }
            { // bottom wall
                const auto & start= trapeze.left_bottom->origin->coords;
                const auto & end= trapeze.right_bottom->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::bottom_wall ||
                                       (a_is_boundary_vertex && (a==start || a==end));
                const bool b_right_of_wall = codes.bottom_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    clamp(b.x, start.x, end.x);
                    b.y= evaluate_line_at_x(b.x, start, end);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::bottom_wall;
                }
            }
            { // top wall
                const auto & start= trapeze.right_top->origin->coords;
                const auto & end= trapeze.left_top->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::top_wall ||
                                       (a_is_boundary_vertex && (a==start || a==end));
                const bool b_right_of_wall = codes.top_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    clamp(b.x, end.x, start.x);
                    b.y= evaluate_line_at_x(b.x, end, start);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::top_wall;
                }
            }

            return point_class_with_trapeze::outside;
        }

        template<typename number>
        auto planarize_division<number>::locate_and_classify_point_that_is_already_on_trapeze(const vertex & point,
                                                                                              const trapeze_t &trapeze)
                                                                                              -> point_class_with_trapeze {
            // given that the point IS present on the trapeze boundary, as opposed to a conflicting point
            // that is classified to belong but was not added yet.
            // this procedure is very ROBUST, and may be optimized later with other methods
            if(point==trapeze.left_top->origin->coords) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.left_bottom->origin->coords) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.right_bottom->origin->coords) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.right_top->origin->coords) return point_class_with_trapeze::boundary_vertex;
            // left wall
            const bool on_left_wall = point.x==trapeze.left_top->origin->coords.x;
            if(on_left_wall) return point_class_with_trapeze::left_wall;
            // right wall
            const bool on_right_wall = point.x==trapeze.right_top->origin->coords.x;
            if(on_right_wall) return point_class_with_trapeze::right_wall;

            auto * start = trapeze.left_bottom;
            auto * end = trapeze.right_bottom;
            while(start!=end) {
                if(point==start->origin->coords)
                    return point_class_with_trapeze::bottom_wall;
                start=start->next;
            }
            start = trapeze.right_top;
            end = trapeze.left_top;
            while(start!=end) {
                if(point==start->origin->coords)
                    return point_class_with_trapeze::top_wall;
                start=start->next;
            }
            return point_class_with_trapeze::unknown;
        }

        template<typename number>
        auto planarize_division<number>::classify_arbitrary_point_with_trapeze(const vertex &point, const trapeze_t &trapeze) -> point_class_with_trapeze {
            const auto codes= compute_location_codes(point, trapeze);
            return classify_from_location_codes(codes);
        }

        template<typename number>
        auto planarize_division<number>::classify_from_location_codes(const location_codes &codes) -> point_class_with_trapeze {
            // given any point, classify it against the location codes, this is robust
            if((codes.left_wall==0 && codes.top_wall==0) || (codes.left_wall==0 && codes.bottom_wall==0) ||
               (codes.right_wall==0 && codes.top_wall==0) || (codes.right_wall==0 && codes.bottom_wall==0))
                return point_class_with_trapeze::boundary_vertex;
            // in closure but not on boundary exterme vertices (as these were resolved earlier)
            const bool in_closure = codes.left_wall>=0 && codes.right_wall>=0 && codes.bottom_wall>=0 && codes.top_wall>=0;
            if(in_closure) { // we are in the closure=boundary or interior
                if(codes.left_wall==0) return point_class_with_trapeze::left_wall;
                if(codes.right_wall==0) return point_class_with_trapeze::right_wall;
                if(codes.bottom_wall==0) return point_class_with_trapeze::bottom_wall;
                if(codes.top_wall==0) return point_class_with_trapeze::top_wall;
                return point_class_with_trapeze::strictly_inside;
            }
            return point_class_with_trapeze::outside;
        }

        template<typename number>
        auto planarize_division<number>::compute_location_codes(const vertex & point, const trapeze_t &trapeze) -> location_codes {
            // reminder: 0=on wall, -1=right of wall, 1= left of wall
            const number pre_left_wall = point.x-trapeze.left_top->origin->coords.x;
            const number pre_right_wall = -point.x+trapeze.right_top->origin->coords.x;
            const int left_wall= pre_left_wall<number(0) ? -1 : (pre_left_wall>number(0) ? 1 : 0);
            const int right_wall= pre_right_wall<number(0) ? -1 : (pre_right_wall>number(0) ? 1 : 0);
            int bottom_wall = classify_point(point, trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords);
            int top_wall = classify_point(point, trapeze.right_top->origin->coords, trapeze.left_top->origin->coords);
            if(point==trapeze.right_top->origin->coords || point==trapeze.left_top->origin->coords) top_wall=0; // this may be more robust and accurate for boundary vertices
            if(point==trapeze.left_bottom->origin->coords || point==trapeze.right_bottom->origin->coords) bottom_wall=0; // this may be more robust and accurate for boundary vertices
            return {left_wall, bottom_wall, right_wall, top_wall};
        }

        template<typename number>
        auto planarize_division<number>::try_split_edge_at(const vertex& point,
                                                           half_edge *edge,
                                                           dynamic_pool & pool) -> half_edge * {
            // let's shorten both edge and it's twin,each from_sampler it's side
            // main frame does not have twins because are needed.
            //  ------edge----->
            //        ===
            //  --e0---*--e1--->
            // <--et1--*--et0--
            // if the point is already one of the edges endpoints skip fast
            if(point==edge->origin->coords) return edge;
            if(point==edge->twin->origin->coords) return edge->twin;

            auto * e_0 = edge;
            auto * e_1 = pool.create_edge();
            auto * e_t_0 = edge->twin;
            auto * e_t_1 = pool.create_edge();

            e_1->origin = pool.create_vertex(point);
            e_1->prev = e_0;
            e_1->next = e_0->next;
            e_1->next->prev = e_1;
            e_0->next = e_1;
            // inherit face and winding properties
            e_1->face = e_0->face;
            e_1->winding = e_0->winding;
            // twins
            e_t_1->origin = e_1->origin;
            e_t_1->prev = e_t_0;
            e_t_1->next = e_t_0->next;
            e_t_1->next->prev = e_t_1;
            e_t_0->next = e_t_1;
            // inherit face and winding properties
            e_t_1->face = e_t_0->face;
            e_t_1->winding = e_t_0->winding;
            // connect twins
            e_t_1->twin = e_0;
            e_0->twin = e_t_1;
            e_t_0->twin = e_1;
            e_1->twin = e_t_0;
            // make sure point refers to any edge leaving it
            e_1->origin->edge = e_1;

            return e_1;
        }

        template<typename number>
        auto planarize_division<number>::try_insert_vertex_on_trapeze_boundary_at(const vertex & v,
                                                                                  const trapeze_t & trapeze,
                                                                                  point_class_with_trapeze where_boundary,
                                                                                  dynamic_pool & pool) -> half_edge * {
            // given where on the boundary: left, top, right, bottom
            // walk along that boundary ray and insert the vertex at the right place.
            // if the vertex already exists, return it's corresponding half edge.
            // otherwise, insert a vertex and split the correct edge segment of the ray.
            // at the end, return the corresponding half-edge, whose origin is the vertex.
            half_edge * e_start = nullptr;
            half_edge * e_end = nullptr;
            bool compare_y = false;
            bool reverse_direction = false;

            switch (where_boundary) {
                case point_class_with_trapeze::strictly_inside:
                    return nullptr;
                case point_class_with_trapeze::boundary_vertex:
                    if(v==trapeze.left_top->origin->coords) return trapeze.left_top;
                    if(v==trapeze.left_bottom->origin->coords) return trapeze.left_bottom;
                    if(v==trapeze.right_bottom->origin->coords) return trapeze.right_bottom;
                    if(v==trapeze.right_top->origin->coords) return trapeze.right_top;
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
#if DEBUG_PLANAR==true
                    throw std::runtime_error("try_insert_vertex_on_trapeze_boundary_at:: wall class is not boundary !!!");
#endif
                    return nullptr;
            }

            const auto * e_end_ref = e_end;
            do {
                const auto coord_0 = compare_y ? e_start->origin->coords.y : e_start->origin->coords.x;
                const auto coord_1 = compare_y ? e_start->next->origin->coords.y : e_start->next->origin->coords.x;
                const auto v_coord = compare_y ? v.y : v.x;
                const bool on_segment = reverse_direction ? v_coord>coord_1 : v_coord<coord_1;
                // check endpoints first
                if(v_coord==coord_0) return e_start;
                if(on_segment)
                    return try_split_edge_at(v, e_start, pool);
                e_start=e_start->next;
            } while(e_start!=e_end_ref);

            return nullptr;
        }

        template<typename number>
        number planarize_division<number>::evaluate_line_at_x(const number x, const vertex &a, const vertex &b) {
            if(x==a.x) return a.y;
            if(x==b.x) return b.y;
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

            // clear the ref from_sampler vertex to face
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
                auto * current_ref = list_ref;  // record head
                list_ref=list_ref->next;        // advance
                current_ref->next = nullptr;    // de-attach
                auto * e = current_ref->edge;
                // find the face to which the edge is classified
                auto * f = classify_conflict_against_two_faces(face_separator, e);
                // insert edge into head of conflict list
                current_ref->next = f->conflict_list;
                f->conflict_list = current_ref;
                // pointer from_sampler edge to conflicting face
                e->conflict_face=f;
            }
        }

        template<typename number>
        auto planarize_division<number>::classify_conflict_against_two_faces(const half_edge* face_separator,
                                                                             const half_edge* edge) -> half_edge_face *{
            // note:: edge's face always points to the face that lies to it's left.
            // 1. if the first point lie completely to the left of the edge, then they belong to f1, other wise f2
            // 2. if they lie exactly on the edge, then we test the second end-point
            // 2.1 if the second lies completely in f1, then the first vertex is in f1, otherwise f2
            // 2.2 if the second lies on the face_separator as well, choose whoever face you want, let's say f1
            const auto & a = face_separator->origin->coords;
            const auto & b = face_separator->twin->origin->coords;
            const auto & c = edge->origin->coords;
            const auto & d = edge->twin->origin->coords;
            const int cls = classify_point(c, a, b);

            if(cls>0) // if strictly left of
                return face_separator->face; // face points always to the face left of edge
            else if(cls<0)// if strictly right of
                return face_separator->twin->face;
            else { // is exactly on separator
                const int cls2 = classify_point(d, a, b);
                if(cls2>0)// if strictly left of
                    return face_separator->face;
                else if(cls2<0)// if strictly right of
                    return face_separator->twin->face;
            }
            // both lie on the separator
            return face_separator->face;
        }

        template<typename number>
        void planarize_division<number>::walk_and_update_edges_face(half_edge * edge_start, half_edge_face * face) {
            // start walk at edge and update all face references.
            auto * e_ref = edge_start;
            const auto * const e_end = edge_start;
            do {
                e_ref->face = face;
                e_ref= e_ref->next;
            } while(e_ref!=e_end);
        }

        template<typename number>
        bool planarize_division<number>::is_a_before_or_equal_b_on_same_boundary(const vertex &a, const vertex &b,
                                                                                 const point_class_with_trapeze &wall) {
            switch (wall) {
                case point_class_with_trapeze::left_wall: return (a.y<=b.y);
                case point_class_with_trapeze::right_wall: return (b.y<=a.y);
                case point_class_with_trapeze::bottom_wall: return (a.x<=b.x);
                case point_class_with_trapeze::top_wall: return (b.x<=a.x);
                default: // this might be problematic, we may need to report error
#if DEBUG_PLANAR==true
                    throw std::runtime_error("is_a_before_or_equal_b_on_same_boundary:: unrecognized boundary");
#endif
                    return false;
            }
        }

        template<typename number>
        auto planarize_division<number>::locate_half_edge_of_face_rooted_at_vertex(const half_edge_vertex *root,
                                                                                   const half_edge_face * face) -> half_edge * {
            auto *iter = root->edge;
            const auto * const end = root->edge;
            do {
                if(iter->face==face) return iter;
                iter=iter->twin->next;
            } while(iter!=end);

            return nullptr;
        }

        template<typename number>
        auto planarize_division<number>::locate_face_of_a_b(const half_edge_vertex &a,
                                                            const vertex &b) -> half_edge * {
            // given edge (a,b) as half_edge_vertex a and a vertex b, find out to which
            // adjacent face does this edge should belong. we return the half_edge that
            // has this face to it's left and vertex 'a' as an origin. we walk CW around
            // the vertex to find which subdivision. The reason that the natural order around
            // a vertex is CW is BECAUSE, that the face edges are CCW. If one draws it on paper
            // then it will become super clear.
            half_edge *iter = a.edge;
            const half_edge *end = iter;
            do { // we walk in CCW order around the vertex
                int cls1= classify_point(b, iter->origin->coords,
                                         iter->twin->origin->coords);
                auto* next = iter->twin->next;
                int cls2= classify_point(b, next->origin->coords,
                                         next->twin->origin->coords);
                // is (a,b) inside the cone so right of iter and left of next
                if(cls1<=0 && cls2>0)
                    return next;

                iter=iter->twin->next;
            } while(iter!=end);

            return iter;
        }

        template<typename number>
        auto planarize_division<number>::insert_edge_between_non_co_linear_vertices(half_edge *vertex_a_edge,
                                                                                    half_edge *vertex_b_edge,
                                                                                    dynamic_pool & pool) -> half_edge * {
            // insert edge between two vertices in a face, that are not co linear with one of the 4 walls, located
            // by their leaving edges. co linearity means the vertices lie on the same boundary ray.
            // for that case, we have a different procedure to handle.
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
            face->edge= e;
            face_2->edge= e_twin;
            // now, iterate on all of the conflicts of f, and move the correct ones to f2
            re_distribute_conflicts_of_split_face(e->face->conflict_list, e);
            // all edges of face_2 point to the old face_1, we need to change that
            walk_and_update_edges_face(e_twin, face_2);
            return e;
        }

        template<typename number>
        void planarize_division<number>::handle_co_linear_edge_with_trapeze(const trapeze_t &trapeze,
                                                                            const vertex & a,
                                                                            const vertex & b,
                                                                            const point_class_with_trapeze &wall_class, // left/top/right/bottom only
                                                                            half_edge ** result_edge_a,
                                                                            half_edge ** result_edge_b,
                                                                            dynamic_pool& pool) {
            // given that (a) and (b) are on the boundary and edge (a,b) is co-linear with one of the 4 boundaries of the trapeze,
            // try inserting vertices on that boundary, update windings along their path/between them and return the
            // corresponding start and end half edges whose start points are (a) and (b)
            auto * edge_vertex_a =
                    try_insert_vertex_on_trapeze_boundary_at(a, trapeze, wall_class, pool);
            auto * edge_vertex_b =
                    try_insert_vertex_on_trapeze_boundary_at(b, trapeze, wall_class, pool);
            // this is a dangerous practice, would be better to return a struct with pointers
            if(result_edge_a) *result_edge_a = edge_vertex_a;
            if(result_edge_b) *result_edge_b = edge_vertex_b;

            const int winding = infer_edge_winding(a, b);
            if(winding==0) return;

            auto *start = edge_vertex_a;
            auto *end = edge_vertex_b;
            // sort if needed
            bool before = is_a_before_or_equal_b_on_same_boundary(a, b, wall_class);
            if(!before) { auto *temp=start;start=end;end=temp; }
            while(start!=end) {
                start->winding+=winding;
                start->twin->winding=start->winding;
                start=start->next;
            }

        }

        template<typename number>
        auto planarize_division<number>::handle_vertical_face_cut(const trapeze_t &trapeze, vertex & a,
                                                                  const point_class_with_trapeze &a_classs,
                                                                  dynamic_pool & dynamic_pool) -> vertical_face_cut_result {
            // the procedure returns the half edge of the face for which it's origin vertex
            // is the insertion point. the half_edge_vertex *a will be inserted in place if it didn't exist yet.
            // the mission of this method is to split a little to create an intermediate face
            // for the regular procedure, this procedure might split a face to two or just
            // split an edge on which the vertex lies on.
            // todo: what about degenerate trapeze with three vertices ?
            // given that vertex (a) is in the closure of the trapeze
            // two things can occur, the vertex is completely inside the trapeze
            // 1. the vertex is strictly inside the horizontal part of the trapeze,
            //    therefore, add a vertical segment that cuts the top/bottom segments
            // 1.1. this also induces a splitting of the face into two faces including updating
            //      references and conflicts
            // 1.2. if the vertex was completely inside the trapeze(not on the top/bottom boundary),
            //      then split that vertical line as well. Note, that the vertical line is unique because
            //      of the strict horizontal inclusion.
            // 2. the vertex is on the left/right walls (including endpoints, i.e not necessarily strict inside),
            //    in this case just split the vertical left or right walls vertically by inserting a vertex if not exist already.
            const bool on_boundary = a_classs != point_class_with_trapeze::strictly_inside;
            const bool on_boundary_vertices = a_classs == point_class_with_trapeze::boundary_vertex;
            const bool in_left_wall = a_classs == point_class_with_trapeze::left_wall;
            const bool in_right_wall = a_classs == point_class_with_trapeze::right_wall;
            const bool in_top_wall = a_classs == point_class_with_trapeze::top_wall;
            const bool should_try_split_horizontal_trapeze_parts = !in_left_wall && !in_right_wall && !on_boundary_vertices;
            vertical_face_cut_result result{};
            result.left_trapeze = result.right_trapeze = trapeze;
            // vertical edge that a sits on
            half_edge * outgoing_vertex_edge=nullptr;
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
                // edge cannot exist yet because we are strictly inside horizontal part.
                // we insert a vertical edge, that starts at bottom edge into the top wall (bottom-to-top)
                auto *start_vertical_wall = insert_edge_between_non_co_linear_vertices(bottom_vertex_edge,
                                                                                       top_vertex_edge,dynamic_pool);
                // clamp vertex to this new edge endpoints if it is before or after
                // this fights the geometric numeric precision errors, that can happen in y coords
                clamp_vertex_horizontally(a, start_vertical_wall->origin->coords, start_vertical_wall->twin->origin->coords);
                clamp_vertex_vertically(a, start_vertical_wall->origin->coords, start_vertical_wall->twin->origin->coords);
                // update resulting trapezes
                result.left_trapeze.right_bottom = start_vertical_wall;
                result.left_trapeze.right_top = start_vertical_wall->next;
                result.right_trapeze.left_top = start_vertical_wall->twin;
                result.right_trapeze.left_bottom = start_vertical_wall->twin->next;
                // if the vertex is on the edge boundary, it will not split of course
                outgoing_vertex_edge = try_split_edge_at(a, start_vertical_wall, dynamic_pool);
            } // else we are on left or right walls already
            else // we are on left or right boundary
                outgoing_vertex_edge=try_insert_vertex_on_trapeze_boundary_at(a, trapeze, a_classs, dynamic_pool);

            result.face_was_split = should_try_split_horizontal_trapeze_parts;
            result.vertex_a_edge_split_edge = outgoing_vertex_edge;

            return result;
        }

        template<typename number>
        auto planarize_division<number>::handle_face_split(const trapeze_t & trapeze,
                                                           vertex &a, vertex &b,
                                                           const point_class_with_trapeze &a_class,
                                                           const point_class_with_trapeze &b_class,
                                                           dynamic_pool & dynamic_pool) -> half_edge * {
            // given that edge (a,b) should split the face, i.e inside the face and is NOT
            // co-linear with a boundary wall of trapeze (for that case we have another procedure),
            // split the face to up to 4 pieces, strategy is:
            // 1. use a vertical that goes through "a" to try and split the trapeze
            // 2. if a cut has happened, two trapezes were created and "a" now lies on
            //    the vertical wall between them.
            // 3. infer the correct trapeze where "b" is in and perform roughly the same
            //    operation
            // 4. connect the two points, so they again split a face (in case they are not vertical)

            const int winding = infer_edge_winding(a, b);
            // first, in order to avoid robust issues later, we insert the (b) vertex on the trapeze.
            // if (b) is not strictly inside, then it is on the boundary of the trapeze, so let's
            // insert the vertex ASAP before the cuts begin
            const bool b_is_on_original_boundary = b_class!=point_class_with_trapeze::strictly_inside;
            if(b_is_on_original_boundary)
                try_insert_vertex_on_trapeze_boundary_at(b, trapeze, b_class, dynamic_pool);
            // first, try to split vertically with endpoint (a)
            vertical_face_cut_result a_cut_result = handle_vertical_face_cut(trapeze,
                                                                             a, a_class, dynamic_pool);
            // let's find out where (b) ended up after a split (that might have not happened)
            const trapeze_t * trapeze_of_b = &trapeze;
            point_class_with_trapeze b_new_class = b_class;
            if(a_cut_result.face_was_split) {
                // a vertical split happened, let's see where b is
                bool left = b.x<=a_cut_result.vertex_a_edge_split_edge->origin->coords.x;
                trapeze_of_b = left ? &a_cut_result.left_trapeze : &a_cut_result.right_trapeze;
                // now, we need to update vertex (b) class after the vertical (a) cut in a robust manner.
                // - If b was on boundary, then it is still on a boundary but might change to a boundary_vertex class
                //   this happens if a.x==b.x
                // - if b had 'strictly_inside' class, then it stays the same unless a.x==b.x and then it
                //   changes class into left/right wall
                {
                    if(a.x==b.x) {
                        if(b_is_on_original_boundary)
                            b_new_class= point_class_with_trapeze::boundary_vertex;
                        else { // b was strictly inside in original trapeze
                            b_new_class = trapeze_of_b->left_bottom->origin->coords.x == b.x ?
                                          point_class_with_trapeze::left_wall :
                                          point_class_with_trapeze::right_wall;
                        }
                    }
                }
            }

            // next, try to split trapeze_of_b vertically with b endpoint
            vertical_face_cut_result b_cut_result = handle_vertical_face_cut(*trapeze_of_b,
                                                                             b, b_new_class, dynamic_pool);
            const trapeze_t *mutual_trapeze = trapeze_of_b;
            // now, infer the mutual trapeze of a and b after the vertical splits have occured
            if(b_cut_result.face_was_split) {
                // a vertical split happened, let's see where b is
                bool left = a.x<=b_cut_result.vertex_a_edge_split_edge->origin->coords.x;
                mutual_trapeze = left ? &b_cut_result.left_trapeze : &b_cut_result.right_trapeze;
            }

            // two options ?
            // 1. (a,b) is vertical, need to only update it's winding
            // 2. (a,b) is NOT vertical, connect a new edge between them, split
            // 3. BUT, first we need to infer the correct half edges, that go out of a and b
            //    and have the mutual face to their left

            // any edge of the trapeze of b will tell us the face
            const auto * face = mutual_trapeze->right_bottom->face;
            // a_edge/b_edge both are half edges rooted at a/b respectively and such that both belong to same face
            auto * a_edge = locate_half_edge_of_face_rooted_at_vertex(a_cut_result.vertex_a_edge_split_edge->origin, face);
            auto * b_edge = locate_half_edge_of_face_rooted_at_vertex(b_cut_result.vertex_a_edge_split_edge->origin, face);
            auto * inserted_edge = a_edge;
            bool is_a_b_vertical = a_edge->origin->coords.x==b_edge->origin->coords.x;
            if(!is_a_b_vertical)
                inserted_edge = insert_edge_between_non_co_linear_vertices(a_edge, b_edge,
                                                                           dynamic_pool);

            inserted_edge->winding += winding;
            inserted_edge->twin->winding = inserted_edge->winding; // a_edge->twin is b_edge

            return inserted_edge;
        }

        template<typename number>
        auto planarize_division<number>::handle_face_merge(const half_edge_vertex *vertex_on_vertical_wall
        ) -> void {
            // given that vertex v is on a past vertical wall (left/right) boundary, we want to check 2 things:
            // 1. the vertical top/bottom edge that adjacent to it, is single(not divided) in it's adjacent trapezes
            // 2. has winding==0
            // 3. horizontal (top/bottom) wall that touches it from_sampler both side is almost a line
            // if all the above happens then we can remove that edge and maintain a variant of the contraction principle.
            // now, try to test if we can shrink the top or bottom or both edges adjacent to the vertex
            // how we do it: examine upwards/downwards half edge
            // 1. if it has winding!=0 skip
            // 2. if there is another edge after it on the vertical wall, skip as well
            // 3. examine how close the horizontal top/bottom edges that lie at the end
            // 4. if they are close than an epsilon, remove the vertical edge
            // 4.1. concatenate the conflicting edges list
            // 4.2. walk on the right face's edges and re-point all half edges to the left face
            const half_edge_vertex *v = vertex_on_vertical_wall;
            half_edge * candidates[2] = {nullptr, nullptr};
            int index=0;
            half_edge *iter = v->edge;
            half_edge *top_edge=nullptr, *bottom_edge=nullptr;
            const half_edge *end = iter;
            do { // we walk in CCW order around the vertex
                bool is_vertical_edge = iter->twin->origin->coords.x==v->coords.x;
                if(is_vertical_edge) {
                    bool is_top = iter->twin->origin->coords.y < v->coords.y; // equality cannot happen
                    if(is_top)
                        top_edge= iter;
                    else
                        bottom_edge= iter;

                    candidates[index++]= iter;
                }

                if(index==2)
                    break;

                iter=iter->twin->next;
            } while(iter!=end);

            // there have to be top and bottom because v was strictly inside the wall
            if(top_edge== nullptr || bottom_edge== nullptr) {
#if DEBUG_PLANAR==true
                throw std::runtime_error("handle_face_merge::have not found bottom or top edge !!!");
#endif
                return;
            }

            // iterate over up to two candidates
            for (int ix = 0; ix < index; ++ix) {
                auto * candidate_edge = candidates[ix];
                if(!candidate_edge)
                    break;

                // start with top edge
                // perform test 1 & 2
                bool valid = candidate_edge->winding==0 &&
                             (candidate_edge->next->twin->origin->coords.x!=candidate_edge->origin->coords.x);
                if(valid) {
                    // top edge goes up and CCW in it's face
                    // a___b___c
                    //     |
                    // ----v----
                    //     |
                    // c'__b'__a'
                    // now test how much is a-b-c looks like a line
                    // we do it by calculating the distance from_sampler vertex c to line (a,b)
                    // the illustration above is for top_edge= v-->b
                    const auto & a = candidate_edge->next->twin->origin->coords;
                    const auto & b = candidate_edge->twin->origin->coords;
                    const auto & c = candidate_edge->twin->prev->origin->coords;
                    // perform test #3
                    bool is_abc_almost_a_line = is_distance_to_line_less_than_epsilon(c, a, b, number(1));
                    if(is_abc_almost_a_line) {
                        // if it is almost a line, then (v,b) edge eligible for removal
                        remove_edge(candidate_edge);
                    }

                }

            }

        }

        template<typename number>
        bool planarize_division<number>::is_distance_to_line_less_than_epsilon(const vertex &v,
                                                                               const vertex &a, const vertex &b, number epsilon) {
            // we use the equation 2*A = h*d(a,b)
            // where A = area of triangle spanned by (a,b,v), h= distance of v to (a,b)
            // we raise everything to quads to avoid square function and we avoid division.
            // while this is more robust, you have to make sure that your number type will
            // not overflow (use 64 bit for fixed point integers)
            number numerator= abs((b.x - a.x) * (v.y - a.y) - (v.x - a.x) * (b.y - a.y)); // 2*A
            number numerator_quad = numerator*numerator; // (2A)^2
            number ab_length_quad = (b.y - a.y)*(b.y - a.y) + (b.x - a.x)*(b.x - a.x); // (length(a,b))^2
            number epsilon_quad = epsilon*epsilon;
            return numerator_quad < epsilon_quad*ab_length_quad;
        }

        template<typename number>
        void planarize_division<number>::remove_edge(half_edge *edge) {
            // remove an edge and it's twin, then:
            // 1.  re-connect adjacent edges
            // 2.  merge faces
            // 3.  move face_2's conflict list into face_1
            auto * face_1 = edge->face;
            auto * face_2 = edge->twin->face;
            // update right adjacent face edges' to point left face
            walk_and_update_edges_face(edge->twin, edge->face);
            // re-connect start segment
            edge->prev->next = edge->twin->next;
            edge->twin->next->prev = edge->prev;
            // re-connect end segment
            edge->twin->prev->next = edge->next;
            edge->next->prev= edge->twin->prev;
            // update the vertices edge links
            edge->origin->edge= edge->twin->next;
            edge->twin->origin->edge= edge->next;
            // move face_2's conflict list into face_1
            auto * conflict_ref= face_1->conflict_list;
            // move the pointer to the last conflict of face_1
            if(conflict_ref) {
                while (conflict_ref->next!=nullptr)
                    conflict_ref=conflict_ref->next;
                // hook the last conflict of face_1 to the first conflict of face_2
                conflict_ref->next = face_2->conflict_list;
            } else {
                face_1->conflict_list=conflict_ref = face_2->conflict_list;
            }
            // now update the conflicting edges with the correct face
            while (conflict_ref!=nullptr) {
                conflict_ref->edge->conflict_face=face_1;
                conflict_ref=conflict_ref->next;
            }
            face_2->edge=nullptr; face_2->conflict_list= nullptr;
        }

        template<typename number>
        void planarize_division<number>::insert_edge(half_edge *edge, index idx, dynamic_pool & dynamic_pool) {
            // start with the conflicting face and iterate carefully, a---b'---b
            bool are_we_done = false;
            int count = 0;
            vertex a, b, b_tag;
            half_edge_face * face = edge->conflict_face;
            point_class_with_trapeze wall_result;
            trapeze_t trapeze=infer_trapeze(face);
            point_class_with_trapeze class_a= round_vertex_to_trapeze(edge->origin->coords, trapeze);
            a=edge->origin->coords;
            b=edge->twin->origin->coords;
            if(a==b) return;

            while(!are_we_done) {
                half_edge *a_vertex_edge= nullptr, *b_tag_vertex_edge= nullptr;
                // the reporting of and class of the next interesting point of the edge against current trapeze
                conflicting_edge_intersection_status edge_status =
                        compute_conflicting_edge_intersection_against_trapeze(trapeze, a, b, class_a);
                b_tag = edge_status.point_of_interest;
                point_class_with_trapeze class_b_tag = edge_status.class_of_interest_point;

#if DEBUG_PLANAR==true
                if(b_tag==a)
                throw std::runtime_error("insert_edge():: a==b_tag, which indicates a problem !!!");
#endif
                // does edge (a,b') is co linear with boundary ? if so treat it
                bool co_linear_with_boundary = do_a_b_lies_on_same_trapeze_wall(trapeze, a, b_tag,
                                                                                class_a, class_b_tag,
                                                                                wall_result);
                if(co_linear_with_boundary) {
                    // co-linear, so let's just insert vertices on the boundary and handle windings
                    // we do not need to split face in this case
                    // Explain:: why don't we send vertical splits lines for boundary ? I think
                    // because the input is always a closed polyline so it doesn't matter
                    handle_co_linear_edge_with_trapeze(trapeze, a, b_tag, wall_result,
                                                       &a_vertex_edge, &b_tag_vertex_edge, dynamic_pool);
                } else {
                    // not co-linear so we have to split the trapeze into up to 4 faces
                    // btw, we at-least split vertically into two top and bottom pieces
                    a_vertex_edge= handle_face_split(trapeze, a, b_tag, class_a, class_b_tag,
                                                     dynamic_pool);
                    b_tag_vertex_edge= a_vertex_edge->twin;

                    if(count==0) {
                        // handle_face_split ,method might change/clamp 'a' vertex. If this is
                        // the first endpoint, we would like to copy this change to source.
                        // this should contribute to robustness
                        edge->origin->coords=a_vertex_edge->origin->coords;
                    }
                }

                // now, we need to merge faces if we split a vertical wall, i.e, if
                // the new 'a' coord strictly lies in the left/right wall
                // record last split vertex if it was on a vertical wall and not the first vertex
                // and not a co-linear segment on the boundary
                bool candidate_merge = (count>=1) && !co_linear_with_boundary &&
                                       (class_a==point_class_with_trapeze::left_wall ||
                                        class_a==point_class_with_trapeze::right_wall);

                if(candidate_merge&&(APPLY_MERGE))
                    handle_face_merge(a_vertex_edge->origin);

                // increment
                // if b'==b we are done
                are_we_done = b==b_tag;
                if(are_we_done)
                    break;
                // todo: if b is the last endpoint of the edge, update it's corrdinates with the original
                // todo: source copy, in case there was a clamping

                // ITERATION STEP, therefore update:
                // 1. now iterate on the sub-line (b', b), by assigning a=b'
                // 2. locate the face, that (b', b) is intruding into / conflicting
                // 3. infer the trapeze of the face
                // 4. infer the class of the updated vertex a=b' in this face
                a =b_tag;
                half_edge * located_face_edge= locate_face_of_a_b(*b_tag_vertex_edge->origin, b);
                face = located_face_edge->face;
                trapeze=infer_trapeze(face);
                class_a = locate_and_classify_point_that_is_already_on_trapeze(a, trapeze);
                count++;
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
                insert_edge(e, ix, dynamic_pool);
            }

        }

        template<typename number>
        void planarize_division<number>::compute_DEBUG(const chunker<vertex> &pieces,
                                                       dynamic_array<vertex> &debug_trapezes) {

            // vertices size is also edges size since these are polygons
            const auto v_size = pieces.unchunked_size();
            // plus 4s for the sake of frame
            static_pool static_pool(v_size + 4, 4*2 + v_size*2, 1, v_size);
            dynamic_pool dynamic_pool{};
            // create the main frame
            auto * main_face = create_frame(pieces, static_pool);
            // create edges and conflict lists
            auto ** edges_list = build_edges_and_conflicts(pieces, *main_face, static_pool);
            // todo: here create a random permutation of edges

            // now start iterations
            for (int ix = 0; ix < v_size; ++ix) {
//        for (int ix = 0; ix < 6; ++ix) {
                auto * e = edges_list[ix];

                //remove_edge_from_conflict_list(e);
                insert_edge(e, ix, dynamic_pool);
            }

            // collect trapezes so far
            face_to_trapeze_vertices(main_face, debug_trapezes);
            auto &faces = dynamic_pool.getFaces();
            int count_active_faces= 0;
            for (index ix = 0; ix < faces.size(); ++ix) {
                if(faces[ix]->edge!= nullptr)
                    count_active_faces++;
                face_to_trapeze_vertices(faces[ix], debug_trapezes);
            }

            std::cout<< "# active faces: " << count_active_faces <<std::endl;
        }

        template<typename number>
        void planarize_division<number>::clamp(number &val, const number & a, const number &b) {
            if(val<a) val = a;
            if(val>b) val = b;
        }

        template<typename number>
        void planarize_division<number>::clamp_vertex(vertex &v, vertex a, vertex b) {
            bool is_a_before_b = a.x<b.x || (a.x==b.x && a.y<b.y);
            if(!is_a_before_b) { vertex c=a;a=b;b=c; } // sort, so a is before b
            bool is_v_before_a = v.x<a.x || (v.x==a.x && v.y<a.y);
            if(is_v_before_a) v=a;
            bool is_v_after_b = v.x>b.x || (v.x==b.x && v.y>=b.y);
            if(is_v_after_b) v=b;
        }

        template<typename number>
        void planarize_division<number>::clamp_vertex_horizontally(vertex &v, vertex a, vertex b) {
            if(a.x>b.x) { vertex c=a;a=b;b=c; }
            if(v.x<a.x) v=a;
            if(v.x>b.x) v=b;
        }

        template<typename number>
        void planarize_division<number>::clamp_vertex_vertically(vertex &v, vertex a, vertex b) {
            if(a.y>b.y) { vertex c=a;a=b;b=c; }
            if(v.y<a.y) v=a;
            if(v.y>b.y) v=b;
        }

        template<typename number>
        bool planarize_division<number>::is_trapeze_degenerate(const trapeze_t & trapeze) {
            return (trapeze.left_top->origin==trapeze.left_bottom->origin) ||
                   (trapeze.right_top->origin==trapeze.right_bottom->origin);
        }

        template<typename number>
        int planarize_division<number>::infer_edge_winding(const vertex & a, const vertex & b) {
            // infer winding of edge (a,b)
            if(b.y<a.y) return 1; // rising/ascending edge
            if(b.y>a.y) return -1; // descending edge
            return 0;
        }

        template<typename number>
        bool planarize_division<number>::do_a_b_lies_on_same_trapeze_wall(const trapeze_t & trapeze,
                                                                          const vertex &a,
                                                                          const vertex &b,
                                                                          const point_class_with_trapeze & a_class,
                                                                          const point_class_with_trapeze & b_class,
                                                                          point_class_with_trapeze &resulting_wall) {
            // given an edge (a,b) where both a and b belong to trapeze, test if the edge is co-linear with one of the
            // boundaries of the trapeze. we classify symbolically rather than analytically in order to be robust.
            // this procedure also handles degenerate trapezes. if true, then resulting_wall will indicate which wall
            // can be one of four: left_wall, right_wall, top_wall, bottom_wall
            // skip inside
            if(a_class==point_class_with_trapeze::strictly_inside ||
               b_class==point_class_with_trapeze::strictly_inside)
                return false;

            bool a_on_boundary_vertex = a_class==point_class_with_trapeze::boundary_vertex;
            bool b_on_boundary_vertex = b_class==point_class_with_trapeze::boundary_vertex;
            bool same_class = a_class==b_class;

            // test_texture if on same wall
            if(same_class && !a_on_boundary_vertex && !b_on_boundary_vertex) {
                resulting_wall=a_class;
                return true;
            }
            // todo: what happens when trapeze is degenerate - a triangle
            bool a_on_wall, b_on_wall;
            // test left wall
            a_on_wall = a_class==point_class_with_trapeze::left_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.left_top->origin->coords || a==trapeze.left_bottom->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::left_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.left_top->origin->coords || b==trapeze.left_bottom->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::left_wall;
                return true;
            }

            // test right wall
            a_on_wall = a_class==point_class_with_trapeze::right_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.right_top->origin->coords || a==trapeze.right_bottom->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::right_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.right_top->origin->coords || b==trapeze.right_bottom->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::right_wall;
                return true;
            }

            // test top wall
            a_on_wall = a_class==point_class_with_trapeze::top_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.right_top->origin->coords || a==trapeze.left_top->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::top_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.right_top->origin->coords || b==trapeze.left_top->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::top_wall;
                return true;
            }

            // test bottom wall
            a_on_wall = a_class==point_class_with_trapeze::bottom_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.left_bottom->origin->coords || a==trapeze.right_bottom->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::bottom_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.left_bottom->origin->coords || b==trapeze.right_bottom->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::bottom_wall;
                return true;
            }

            return false;
        }

        template<typename number>
        auto planarize_division<number>::compute_conflicting_edge_intersection_against_trapeze(const trapeze_t & trapeze,
                                                                                               vertex &a, vertex b,
                                                                                               const point_class_with_trapeze & a_class)
        -> conflicting_edge_intersection_status {
            // given that edge (a,b), vertex (a) is conflicting, i.e on boundary or completely inside
            // and we know that the edge passes through the trapeze or lies on the boundary,
            // find the second interesting point, intersection or overlap or completely inside
            // NOTES:
            // ** About (b) vertex
            // 1. because of precision errors, (b) might be completely outside.
            // 2. therefore, if we know the wall of (a) and suppose (b) is right of that wall,
            //    then we need to clamp/round the vertex class
            //
            // ** About (a) vertex
            // 0. vertex (a) is classified to be in the closure of trapeze (even if analytically it might show otherwise)
            // 1. first, if (b) is strictly in or on trapeze boundary, then we are DONE!!
            // 2. although endpoint "a" belongs to the trapeze, there is a chance it might be outside geometrically,
            //    because of numeric precision errors, that occur during vertical splits
            // 3. therefore, we might get two proper intersections for (a,b) edge against
            //    top/bottom/left/right boundaries.
            // 4. therefore, always consider the intersection with the biggest alpha as what we want.
            //

            point_class_with_trapeze cla_b= round_edge_to_trapeze(a, b, a_class, trapeze);
            conflicting_edge_intersection_status result{};
            result.class_of_interest_point = cla_b;
            result.point_of_interest = b;
            // if vertex (b) is inside or on the boundary, we are done
            bool b_in_exterior = cla_b==point_class_with_trapeze::outside;
            bool b_in_interior = cla_b==point_class_with_trapeze::strictly_inside;
            bool b_in_closure = !b_in_exterior;
            bool b_in_boundary = !b_in_interior && !b_in_exterior;
            if(b_in_closure)
                return result;

            // if we got here, vertex (a) is in the closure and vertex (b) is outside/exterior.
            // the strategy:
            // - try to round now the edge from_sampler interior to a wall boundary is they pass a certain threshold
            // - compute intersections against walls and pick the correct one
            // first test if we have b cases
            bool a_is_on_boundary= a_class!=point_class_with_trapeze::strictly_inside;

            // first test if we have a cases
            // we now know, that b is outside, therefore hunt proper intersections
            vertex intersection_point{};
            number alpha(0), alpha1(0);
            number biggest_alpha=number(0);
            intersection_status status;
            bool is_bigger_alpha;

            { // left-wall
                const auto &start = trapeze.left_top->origin->coords;
                const auto &end = trapeze.left_bottom->origin->coords;
                status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                if(status==intersection_status::intersect) {
                    is_bigger_alpha = alpha>=biggest_alpha;
                    if(is_bigger_alpha) {
                        biggest_alpha=alpha;
                        result.class_of_interest_point = point_class_with_trapeze::left_wall;
                        result.point_of_interest = intersection_point;
                        result.point_of_interest.x=start.x;
                        clamp(result.point_of_interest.y, start.y, end.y); // important to clamp for symbolic reasons as well in case of numeric errors.
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                    }
                } else if(status==intersection_status::parallel) {
                    const bool a_on_wall= a.x==start.x;
                    if(a_on_wall) {
                        result.class_of_interest_point = point_class_with_trapeze::left_wall;
                        result.point_of_interest = b;
                        result.point_of_interest.x=start.x;
                        clamp(result.point_of_interest.y, start.y, end.y); // important to clamp for symbolic reasons as well in case of numeric errors.
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                        return result;
                    }

                }

            }
            { // right wall
                const auto &start = trapeze.right_bottom->origin->coords;
                const auto &end = trapeze.right_top->origin->coords;
                status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                if(status==intersection_status::intersect) {
                    is_bigger_alpha = alpha>=biggest_alpha;
                    if(is_bigger_alpha) {
                        biggest_alpha = alpha;
                        result.class_of_interest_point = point_class_with_trapeze::right_wall;
                        result.point_of_interest = intersection_point;
                        result.point_of_interest.x = start.x;
                        clamp(result.point_of_interest.y, end.y, start.y);
                        if (result.point_of_interest == start || result.point_of_interest == end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                    }
                } else if(status==intersection_status::parallel) {
                    const bool a_on_wall= a.x==start.x;
                    if(a_on_wall) {
                        result.class_of_interest_point = point_class_with_trapeze::right_wall;
                        result.point_of_interest = b;
                        result.point_of_interest.x=start.x;
                        clamp(result.point_of_interest.y, end.y, start.y);
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                        return result;
                    }

                }

            }
            { // bottom wall
                const auto &start = trapeze.left_bottom->origin->coords;
                const auto &end = trapeze.right_bottom->origin->coords;
                status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                if(status==intersection_status::intersect) {
                    is_bigger_alpha = alpha>=biggest_alpha;
                    if(is_bigger_alpha) {
                        biggest_alpha=alpha;
                        result.class_of_interest_point = point_class_with_trapeze::bottom_wall;
                        result.point_of_interest = intersection_point;
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                    }

                } else if(status==intersection_status::parallel) {
                    const bool a_on_wall= a==start || a==end || a_class==point_class_with_trapeze::bottom_wall;
                    if(a_on_wall) {
                        result.class_of_interest_point = point_class_with_trapeze::bottom_wall;
                        result.point_of_interest = b;
                        clamp_vertex_horizontally(result.point_of_interest, start, end);
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                        return result;
                    }

                }

            }
            { // top wall
                const auto &start = trapeze.right_top->origin->coords;
                const auto &end = trapeze.left_top->origin->coords;
                status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                if(status==intersection_status::intersect) {
                    is_bigger_alpha = alpha>=biggest_alpha;
                    if(is_bigger_alpha) {
                        biggest_alpha=alpha;
                        result.class_of_interest_point = point_class_with_trapeze::top_wall;
                        result.point_of_interest = intersection_point;
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                    }

                } else if(status==intersection_status::parallel) {
                    const bool a_on_wall= a==start || a==end || a_class==point_class_with_trapeze::top_wall;
                    if(a_on_wall) {
                        result.class_of_interest_point = point_class_with_trapeze::top_wall;
                        result.point_of_interest = b;
                        clamp_vertex_horizontally(result.point_of_interest, start, end);
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                        return result;
                    }

                }

            }

            return result;
        }

        template<typename number>
        auto planarize_division<number>::finite_segment_intersection_test(const vertex &a, const vertex &b,
                                                                          const vertex &c, const vertex &d,
                                                                          vertex & intersection,
                                                                          number &alpha, number &alpha1) -> intersection_status{
            // this procedure will find proper and improper(touches) intersections, but no
            // overlaps, since overlaps induce parallel classification, this would have to be resolved outside
            if(a==b || c==d)
                return intersection_status::degenerate_line;
            auto ab = b - a;
            auto cd = d - c;
            auto dem = ab.x * cd.y - ab.y * cd.x;

            // parallel lines
            if (abs(dem) <= number(1))
                return intersection_status::parallel;
            else {
                auto ca = a - c;
                auto numerator_1 = ca.y*cd.x - ca.x*cd.y;
                auto numerator_2 = -ab.y*ca.x + ab.x*ca.y;

                if (dem > 0) {
                    if (numerator_1 < 0 || numerator_1 > dem ||
                        numerator_2 < 0 || numerator_2 > dem)
                        return intersection_status::none;
                } else {
                    if (numerator_1 > 0 || numerator_1 < dem ||
                        numerator_2 > 0 || numerator_2 < dem)
                        return intersection_status::none;
                }

                // avoid division on trivial edge cases
                if(numerator_1==0) { // a lies on c--d segment
                    alpha=0; intersection = a;
                }
                else if(numerator_1==dem) { // b lies on c--d segment
                    alpha=1; intersection = b;
                }
                else if(numerator_2==0) { // c lies on a--b segment
                    alpha=0; intersection = c;
                }
                else if(numerator_2==dem) { // d lies on a--b segment
                    alpha=1; intersection = d;
                }
                else { // proper intersection
                    alpha = numerator_1/dem;
                    // alpha1 = numerator_2/dem;
//                    intersection = a + ab*alpha;
                    intersection = a + (ab*numerator_1)/dem;
                }
            }

            return intersection_status::intersect;
        }

        template <typename number>
        inline int
        planarize_division<number>::classify_point(const vertex & point, const vertex &a, const vertex & b) {
            // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
            // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))
            //    Input:  three points p, a, b
            //    Return: >0 for P left of the line through a and b
            //            =0 for P  on the line
            //            <0 for P  right of the line
            //    See: Algorithm 1 "Area of Triangles and Polygons"
            auto result= (b.x-a.x)*(point.y-a.y)-(point.x-a.x)*(b.y-a.y);
            if(result<0) return 1;
            else if(result>0) return -1;
            else return 0;
        }

    }
}

```


```c++
namespace microgl {

    namespace tessellation {

        template<typename number>
        auto planarize_division<number>::create_frame(const chunker<vertex> &pieces,
                                                      static_pool & pool) -> half_edge_face * {
            const auto pieces_length = pieces.size();
            vertex left_top=pieces.data()[0];
            vertex right_bottom=left_top;
            // find bbox of all
            for (index ix = 0; ix < pieces_length; ++ix) {
                auto const piece = pieces[ix];
                const auto piece_size = piece.size;
                for (index jx = 0; jx < piece_size; ++jx) {
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
            // extend bbox 10 points
            left_top.x -= number(10);
            left_top.y -= number(10);
            right_bottom.x += number(10);
            right_bottom.y += number(10);
            // vertices
            auto * v0 = pool.get_vertex();
            auto * v1 = pool.get_vertex();
            auto * v2 = pool.get_vertex();
            auto * v3 = pool.get_vertex();
            // coords
            v0->coords = left_top;
            v1->coords = {left_top.x, right_bottom.y};
            v2->coords = right_bottom;
            v3->coords = {right_bottom.x, left_top.y};
            // half edges of vertices
            auto * edge_0 = pool.get_edge();
            auto * edge_1 = pool.get_edge();
            auto * edge_2 = pool.get_edge();
            auto * edge_3 = pool.get_edge();
            // twins of edges
            auto * edge_0_twin = pool.get_edge();
            auto * edge_1_twin = pool.get_edge();
            auto * edge_2_twin = pool.get_edge();
            auto * edge_3_twin = pool.get_edge();
            // connect edges to vertices, CCW from_sampler left-top vertex
            edge_0->origin = v0; edge_0_twin->origin = v1;
            edge_1->origin = v1; edge_1_twin->origin = v2;
            edge_2->origin = v2; edge_2_twin->origin = v3;
            edge_3->origin = v3; edge_3_twin->origin = v0;
            v0->edge = edge_0;
            v1->edge = edge_1;
            v2->edge = edge_2;
            v3->edge = edge_3;
            // winding is zero for frame
            edge_0->winding = edge_0_twin->winding = 0;
            edge_1->winding = edge_1_twin->winding = 0;
            edge_2->winding = edge_2_twin->winding = 0;
            edge_3->winding = edge_3_twin->winding = 0;
            // connect edges among themselves
            edge_0->next = edge_1; edge_0_twin->next = edge_3_twin;
            edge_1->next = edge_2; edge_1_twin->next = edge_0_twin;
            edge_2->next = edge_3; edge_2_twin->next = edge_1_twin;
            edge_3->next = edge_0; edge_3_twin->next = edge_2_twin;
            edge_0->prev = edge_3; edge_0_twin->prev = edge_1_twin;
            edge_1->prev = edge_0; edge_1_twin->prev = edge_2_twin;
            edge_2->prev = edge_1; edge_2_twin->prev = edge_3_twin;
            edge_3->prev = edge_2; edge_3_twin->prev = edge_0_twin;
            // connect twins
            edge_0->twin = edge_0_twin; edge_0_twin->twin = edge_0;
            edge_1->twin = edge_1_twin; edge_1_twin->twin = edge_1;
            edge_2->twin = edge_2_twin; edge_2_twin->twin = edge_2;
            edge_3->twin = edge_3_twin; edge_3_twin->twin = edge_3;
            // create a face and connect it
            auto * face = pool.get_face();
            edge_0->face = face; edge_0_twin->face = nullptr; // nullptr for face is the outside world face marker
            edge_1->face = face; edge_1_twin->face = nullptr;
            edge_2->face = face; edge_2_twin->face = nullptr;
            edge_3->face = face; edge_3_twin->face = nullptr;
            // CCW around face, face is always to the left of the walk
            face->edge = edge_0;

            return face;
        }

        template<typename number>
        auto planarize_division<number>::build_edges_and_conflicts(const chunker<vertex> &pieces,
                                                                   half_edge_face & main_frame,
                                                                   static_pool & pool) -> half_edge ** {
            // given polygons, build their vertices, edges and conflicts
            const auto pieces_length = pieces.size();
            conflict * conflict_first = nullptr;
            const auto v_size = pieces.unchunked_size();
            auto ** edges_list = new half_edge*[v_size];
            index edges_list_counter = 0;

            // build edges and twins, do not make next/prev connections
            for (index ix = 0; ix < pieces_length; ++ix) {
                half_edge * edge_first = nullptr;
                half_edge * edge_last = nullptr;

                auto const piece = pieces[ix];
                const auto piece_size = piece.size;
                for (index jx = 0; jx < piece_size; ++jx) {
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

                    if(conflict_first)
                        c->next = conflict_first;

                    conflict_first = c;
                    edge_last = e;
                }

                // hook the last edge, from_sampler last vertex into first vertex
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
        auto planarize_division<number>::infer_trapeze(const half_edge_face *face) -> trapeze_t {
            if(face->edge==nullptr) {
#if DEBUG_PLANAR==true
                throw std::runtime_error("infer_trapeze()::trying to infer a trapeze of a probably merged/deleted face !!!");
#endif
                return {};
            }

            auto * e = face->edge;
            const auto * e_end = face->edge;
            trapeze_t trapeze;
            trapeze.left_top = trapeze.left_bottom = trapeze.right_bottom = trapeze.right_top = e;
            // we are walking CCW
            do {
                const auto * v = e->origin;
                auto curr_x = v->coords.x;
                auto curr_y = v->coords.y;
                const int ori = classify_point(v->edge->next->origin->coords, v->edge->prev->origin->coords, v->edge->origin->coords);
                const bool is_ccw = ori>=0, is_cw = ori<=0;
                if(true || is_ccw) {
                    if(curr_x < trapeze.left_top->origin->coords.x || (curr_x==trapeze.left_top->origin->coords.x && curr_y < trapeze.left_top->origin->coords.y))
                        trapeze.left_top = e;
                    if(curr_x < trapeze.left_bottom->origin->coords.x || (curr_x == trapeze.left_bottom->origin->coords.x && curr_y > trapeze.left_bottom->origin->coords.y))
                        trapeze.left_bottom = e;
                    if(curr_x > trapeze.right_bottom->origin->coords.x || (curr_x == trapeze.right_bottom->origin->coords.x && curr_y > trapeze.right_bottom->origin->coords.y))
                        trapeze.right_bottom = e;
                    if(curr_x > trapeze.right_top->origin->coords.x || (curr_x == trapeze.right_top->origin->coords.x && curr_y < trapeze.right_top->origin->coords.y))
                        trapeze.right_top = e;
                }
                e=e->next;

            } while(e!=e_end);

            return trapeze;
        }

        template<typename number>
        auto planarize_division<number>::round_vertex_to_trapeze(vertex & point,
                                                                 const trapeze_t &trapeze) -> point_class_with_trapeze {
            // given that point that should belong to trapeze, BUT may not sit properly or outside because
            // of precision errors, geometrically round it into place
            auto codes= compute_location_codes(point, trapeze);
            // boundary vertices
            if((codes.left_wall==0 && codes.top_wall==0) || (codes.left_wall==0 && codes.bottom_wall==0) ||
               (codes.right_wall==0 && codes.top_wall==0) || (codes.right_wall==0 && codes.bottom_wall==0))
                return point_class_with_trapeze::boundary_vertex;
            { // left wall
                const auto & start = trapeze.left_top->origin->coords;
                const auto & end = trapeze.left_bottom->origin->coords;
                if(codes.left_wall<=0) { // outside or on wall
                    point.x= start.x; // truncate x
                    clamp(point.y, start.y, end.y);
                    if(point==start || point==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::left_wall;
                }
            }
            { // right wall
                const auto & start = trapeze.right_bottom->origin->coords;
                const auto & end = trapeze.right_top->origin->coords;
                if(codes.right_wall<=0) {
                    point.x= start.x; // truncate x
                    clamp(point.y, end.y, start.y);
                    if(point==start || point==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::right_wall;
                }
            }
            { // bottom wall
                const auto & start = trapeze.left_bottom->origin->coords;
                const auto & end = trapeze.right_bottom->origin->coords;
                if(codes.bottom_wall<=0) {
                    point.y= evaluate_line_at_x(point.x, start, end);
                    return point_class_with_trapeze::bottom_wall;
                }
            }
            { // top wall
                const auto & start = trapeze.right_top->origin->coords;
                const auto & end = trapeze.left_top->origin->coords;
                if(codes.top_wall<=0) {
                    point.y= evaluate_line_at_x(point.x, end, start);
                    return point_class_with_trapeze::top_wall;
                }
            }

            return point_class_with_trapeze::strictly_inside;
        }

        template<typename number>
        auto planarize_division<number>::round_edge_to_trapeze(const vertex & a, vertex & b,
                                                               const point_class_with_trapeze & class_a,
                                                               const trapeze_t &trapeze)
        -> point_class_with_trapeze {
            // given a trapeze and an edge (a,b), that is supposed to conflict with it, i.e
            // cross it's interior or boundary, assume that due to precision errors, the edge
            // segment may fall outside, therefore geometrically round it INTO the trapeze if needed.
            // we assume, that (a) class is in the closure of the trapeze even if analytic evaluation
            // might show otherwise (due to numeric precision errors).
            // we have two cases:
            // 1. vertex (a) is on the boundary, therefore test if (b) overflows outside on the same
            //    boundary wall as (a). If so, clamp/round correct (b) to fit in trapeze.
            // 2. if vertex (a) is the interior of trapeze, do nothing but classifying vertex (b)
            // NOTE: in any case report vertex (b) class.

            // preliminary assumption checkup, that a is not outside classified
            if(class_a==point_class_with_trapeze::outside) {
#if DEBUG_PLANAR==true
                throw std::runtime_error("round_edge_to_trapeze():: (a) does not belong to interior !!!");
#endif
                return point_class_with_trapeze::unknown;
            }
            bool a_in_interior= class_a==point_class_with_trapeze::strictly_inside;
            bool a_on_boundary= !a_in_interior;

            // easy test if (b) is in the closure of trapeze
            const auto codes= compute_location_codes(b, trapeze);
            const auto class_b = classify_from_location_codes(codes);
            if(class_b!=point_class_with_trapeze::outside) // b in closure, we are done
                return class_b;

            // if we got here, (b) is outside, now let's round things
            // if (a) is in interior, we are done since there is nothing to round, because there is intersection.
            if(a_in_interior)
                return point_class_with_trapeze::outside;

            // this might be redundant
            bool a_is_boundary_vertex = class_a==point_class_with_trapeze::boundary_vertex;
            // vertex (a) is on boundary since it is not outside and not in interior, now begin edge rounding if possible.
            // for each wall, test if vertex (a) is on the wall and that vertex (b) is strictly right of the wall.
            // if so, clamp it along.

            { // left wall
                const auto & start= trapeze.left_top->origin->coords;
                const auto & end= trapeze.left_bottom->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::left_wall ||
                                       (a_is_boundary_vertex && a.x==start.x);
                const bool b_right_of_wall = codes.left_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    b.x= start.x;
                    clamp(b.y, start.y, end.y);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::left_wall;
                }
            }
            { // right wall
                const auto & start= trapeze.right_bottom->origin->coords;
                const auto & end= trapeze.right_top->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::right_wall ||
                                       (a_is_boundary_vertex && a.x==start.x);
                const bool b_right_of_wall = codes.right_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    b.x= start.x;
                    clamp(b.y, end.y, start.y);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::right_wall;
                }
            }
            { // bottom wall
                const auto & start= trapeze.left_bottom->origin->coords;
                const auto & end= trapeze.right_bottom->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::bottom_wall ||
                                       (a_is_boundary_vertex && (a==start || a==end));
                const bool b_right_of_wall = codes.bottom_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    clamp(b.x, start.x, end.x);
                    b.y= evaluate_line_at_x(b.x, start, end);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::bottom_wall;
                }
            }
            { // top wall
                const auto & start= trapeze.right_top->origin->coords;
                const auto & end= trapeze.left_top->origin->coords;
                const bool a_on_wall = class_a==point_class_with_trapeze::top_wall ||
                                       (a_is_boundary_vertex && (a==start || a==end));
                const bool b_right_of_wall = codes.top_wall<=0;
                if(a_on_wall && b_right_of_wall) { // bingo, let's round
                    clamp(b.x, end.x, start.x);
                    b.y= evaluate_line_at_x(b.x, end, start);
                    if(b==start || b==end) return point_class_with_trapeze::boundary_vertex;
                    return point_class_with_trapeze::top_wall;
                }
            }

            return point_class_with_trapeze::outside;
        }

        template<typename number>
        auto planarize_division<number>::locate_and_classify_point_that_is_already_on_trapeze(const vertex & point,
                                                                                              const trapeze_t &trapeze)
                                                                                              -> point_class_with_trapeze {
            // given that the point IS present on the trapeze boundary, as opposed to a conflicting point
            // that is classified to belong but was not added yet.
            // this procedure is very ROBUST, and may be optimized later with other methods
            if(point==trapeze.left_top->origin->coords) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.left_bottom->origin->coords) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.right_bottom->origin->coords) return point_class_with_trapeze::boundary_vertex;
            if(point==trapeze.right_top->origin->coords) return point_class_with_trapeze::boundary_vertex;
            // left wall
            const bool on_left_wall = point.x==trapeze.left_top->origin->coords.x;
            if(on_left_wall) return point_class_with_trapeze::left_wall;
            // right wall
            const bool on_right_wall = point.x==trapeze.right_top->origin->coords.x;
            if(on_right_wall) return point_class_with_trapeze::right_wall;

            auto * start = trapeze.left_bottom;
            auto * end = trapeze.right_bottom;
            while(start!=end) {
                if(point==start->origin->coords)
                    return point_class_with_trapeze::bottom_wall;
                start=start->next;
            }
            start = trapeze.right_top;
            end = trapeze.left_top;
            while(start!=end) {
                if(point==start->origin->coords)
                    return point_class_with_trapeze::top_wall;
                start=start->next;
            }
            return point_class_with_trapeze::unknown;
        }

        template<typename number>
        auto planarize_division<number>::classify_arbitrary_point_with_trapeze(const vertex &point, const trapeze_t &trapeze) -> point_class_with_trapeze {
            const auto codes= compute_location_codes(point, trapeze);
            return classify_from_location_codes(codes);
        }

        template<typename number>
        auto planarize_division<number>::classify_from_location_codes(const location_codes &codes) -> point_class_with_trapeze {
            // given any point, classify it against the location codes, this is robust
            if((codes.left_wall==0 && codes.top_wall==0) || (codes.left_wall==0 && codes.bottom_wall==0) ||
               (codes.right_wall==0 && codes.top_wall==0) || (codes.right_wall==0 && codes.bottom_wall==0))
                return point_class_with_trapeze::boundary_vertex;
            // in closure but not on boundary exterme vertices (as these were resolved earlier)
            const bool in_closure = codes.left_wall>=0 && codes.right_wall>=0 && codes.bottom_wall>=0 && codes.top_wall>=0;
            if(in_closure) { // we are in the closure=boundary or interior
                if(codes.left_wall==0) return point_class_with_trapeze::left_wall;
                if(codes.right_wall==0) return point_class_with_trapeze::right_wall;
                if(codes.bottom_wall==0) return point_class_with_trapeze::bottom_wall;
                if(codes.top_wall==0) return point_class_with_trapeze::top_wall;
                return point_class_with_trapeze::strictly_inside;
            }
            return point_class_with_trapeze::outside;
        }

        template<typename number>
        auto planarize_division<number>::compute_location_codes(const vertex & point, const trapeze_t &trapeze) -> location_codes {
            // reminder: 0=on wall, -1=right of wall, 1= left of wall
            const number pre_left_wall = point.x-trapeze.left_top->origin->coords.x;
            const number pre_right_wall = -point.x+trapeze.right_top->origin->coords.x;
            const int left_wall= pre_left_wall<number(0) ? -1 : (pre_left_wall>number(0) ? 1 : 0);
            const int right_wall= pre_right_wall<number(0) ? -1 : (pre_right_wall>number(0) ? 1 : 0);
            int bottom_wall = classify_point(point, trapeze.left_bottom->origin->coords, trapeze.right_bottom->origin->coords);
            int top_wall = classify_point(point, trapeze.right_top->origin->coords, trapeze.left_top->origin->coords);
            if(point==trapeze.right_top->origin->coords || point==trapeze.left_top->origin->coords) top_wall=0; // this may be more robust and accurate for boundary vertices
            if(point==trapeze.left_bottom->origin->coords || point==trapeze.right_bottom->origin->coords) bottom_wall=0; // this may be more robust and accurate for boundary vertices
            return {left_wall, bottom_wall, right_wall, top_wall};
        }

        template<typename number>
        auto planarize_division<number>::try_split_edge_at(const vertex& point,
                                                           half_edge *edge,
                                                           dynamic_pool & pool) -> half_edge * {
            // let's shorten both edge and it's twin,each from_sampler it's side
            // main frame does not have twins because are needed.
            //  ------edge----->
            //        ===
            //  --e0---*--e1--->
            // <--et1--*--et0--
            // if the point is already one of the edges endpoints skip fast
            if(point==edge->origin->coords) return edge;
            if(point==edge->twin->origin->coords) return edge->twin;

            auto * e_0 = edge;
            auto * e_1 = pool.create_edge();
            auto * e_t_0 = edge->twin;
            auto * e_t_1 = pool.create_edge();

            e_1->origin = pool.create_vertex(point);
            e_1->prev = e_0;
            e_1->next = e_0->next;
            e_1->next->prev = e_1;
            e_0->next = e_1;
            // inherit face and winding properties
            e_1->face = e_0->face;
            e_1->winding = e_0->winding;
            // twins
            e_t_1->origin = e_1->origin;
            e_t_1->prev = e_t_0;
            e_t_1->next = e_t_0->next;
            e_t_1->next->prev = e_t_1;
            e_t_0->next = e_t_1;
            // inherit face and winding properties
            e_t_1->face = e_t_0->face;
            e_t_1->winding = e_t_0->winding;
            // connect twins
            e_t_1->twin = e_0;
            e_0->twin = e_t_1;
            e_t_0->twin = e_1;
            e_1->twin = e_t_0;
            // make sure point refers to any edge leaving it
            e_1->origin->edge = e_1;

            return e_1;
        }

        template<typename number>
        auto planarize_division<number>::try_insert_vertex_on_trapeze_boundary_at(const vertex & v,
                                                                                  const trapeze_t & trapeze,
                                                                                  point_class_with_trapeze where_boundary,
                                                                                  dynamic_pool & pool) -> half_edge * {
            // given where on the boundary: left, top, right, bottom
            // walk along that boundary ray and insert the vertex at the right place.
            // if the vertex already exists, return it's corresponding half edge.
            // otherwise, insert a vertex and split the correct edge segment of the ray.
            // at the end, return the corresponding half-edge, whose origin is the vertex.
            half_edge * e_start = nullptr;
            half_edge * e_end = nullptr;
            bool compare_y = false;
            bool reverse_direction = false;

            switch (where_boundary) {
                case point_class_with_trapeze::strictly_inside:
                    return nullptr;
                case point_class_with_trapeze::boundary_vertex:
                    if(v==trapeze.left_top->origin->coords) return trapeze.left_top;
                    if(v==trapeze.left_bottom->origin->coords) return trapeze.left_bottom;
                    if(v==trapeze.right_bottom->origin->coords) return trapeze.right_bottom;
                    if(v==trapeze.right_top->origin->coords) return trapeze.right_top;
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
#if DEBUG_PLANAR==true
                    throw std::runtime_error("try_insert_vertex_on_trapeze_boundary_at:: wall class is not boundary !!!");
#endif
                    return nullptr;
            }

            const auto * e_end_ref = e_end;
            do {
                const auto coord_0 = compare_y ? e_start->origin->coords.y : e_start->origin->coords.x;
                const auto coord_1 = compare_y ? e_start->next->origin->coords.y : e_start->next->origin->coords.x;
                const auto v_coord = compare_y ? v.y : v.x;
                const bool on_segment = reverse_direction ? v_coord>coord_1 : v_coord<coord_1;
                // check endpoints first
                if(v_coord==coord_0) return e_start;
//                if(v_coord==coord_0) { // override
//                    e_start->origin->coords=v;
//                    return e_start;
//                }
//                if(v==e_start->origin->coords)
//                    return e_start;
                if(on_segment)
                    return try_split_edge_at(v, e_start, pool);
                e_start=e_start->next;
            } while(e_start!=e_end_ref);

            return nullptr;
        }

        template<typename number>
        number planarize_division<number>::evaluate_line_at_x(const number x, const vertex &a, const vertex &b) {
            if(x==a.x) return a.y;
            if(x==b.x) return b.y;
            // can be made better for Q numbers
            auto slope = (b.y-a.y)/(b.x-a.x);
            auto y = a.y + slope*(x-a.x);
            return y;
        }

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
                auto * current_ref = list_ref;  // record head
                list_ref=list_ref->next;        // advance
                current_ref->next = nullptr;    // de-attach
                auto * e = current_ref->edge;
                // find the face to which the edge is classified
                auto * f = classify_conflict_against_two_faces(face_separator, e);
                // insert edge into head of conflict list
                current_ref->next = f->conflict_list;
                f->conflict_list = current_ref;
                // pointer from_sampler edge to conflicting face
                e->conflict_face=f;
            }
        }

        template<typename number>
        auto planarize_division<number>::classify_conflict_against_two_faces(const half_edge* face_separator,
                                                                             const half_edge* edge) -> half_edge_face *{
            // note:: edge's face always points to the face that lies to it's left.
            // 1. if the first point lie completely to the left of the edge, then they belong to f1, other wise f2
            // 2. if they lie exactly on the edge, then we test the second end-point
            // 2.1 if the second lies completely in f1, then the first vertex is in f1, otherwise f2
            // 2.2 if the second lies on the face_separator as well, choose whoever face you want, let's say f1
            const auto & a = face_separator->origin->coords;
            const auto & b = face_separator->twin->origin->coords;
            const auto & c = edge->origin->coords;
            const auto & d = edge->twin->origin->coords;
            const int cls = classify_point(c, a, b);

            if(cls>0) // if strictly left of
                return face_separator->face; // face points always to the face left of edge
            else if(cls<0)// if strictly right of
                return face_separator->twin->face;
            else { // is exactly on separator
                const int cls2 = classify_point(d, a, b);
                if(cls2>0)// if strictly left of
                    return face_separator->face;
                else if(cls2<0)// if strictly right of
                    return face_separator->twin->face;
            }
            // both lie on the separator
            return face_separator->face;
        }

        template<typename number>
        void planarize_division<number>::walk_and_update_edges_face(half_edge * edge_start, half_edge_face * face) {
            // start walk at edge and update all face references.
            auto * e_ref = edge_start;
            const auto * const e_end = edge_start;
            do {
                e_ref->face = face;
                e_ref= e_ref->next;
            } while(e_ref!=e_end);
        }

        template<typename number>
        bool planarize_division<number>::is_a_before_or_equal_b_on_same_boundary(const vertex &a, const vertex &b,
                                                                                 const point_class_with_trapeze &wall) {
            switch (wall) {
                case point_class_with_trapeze::left_wall: return (a.y<=b.y);
                case point_class_with_trapeze::right_wall: return (b.y<=a.y);
                case point_class_with_trapeze::bottom_wall: return (a.x<=b.x);
                case point_class_with_trapeze::top_wall: return (b.x<=a.x);
                default: // this might be problematic, we may need to report error
#if DEBUG_PLANAR==true
                    throw std::runtime_error("is_a_before_or_equal_b_on_same_boundary:: unrecognized boundary");
#endif
                    return false;
            }
        }

        template<typename number>
        auto planarize_division<number>::locate_half_edge_of_face_rooted_at_vertex(const half_edge_vertex *root,
                                                                                   const half_edge_face * face) -> half_edge * {
            auto *iter = root->edge;
            const auto * const end = root->edge;
            do {
                if(iter->face==face) return iter;
                iter=iter->twin->next;
            } while(iter!=end);

#if DEBUG_PLANAR==true
            throw std::runtime_error("locate_half_edge_of_face_rooted_at_vertex::not found face !!!");
#endif

            return nullptr;
        }

        template<typename number>
        auto planarize_division<number>::locate_face_of_a_b(const half_edge_vertex &a,
                                                            const vertex &b) -> half_edge * {
            // given edge (a,b) as half_edge_vertex a and a vertex b, find out to which
            // adjacent face does this edge should belong. we return the half_edge that
            // has this face to it's left and vertex 'a' as an origin. we walk CW around
            // the vertex to find which subdivision. The reason that the natural order around
            // a vertex is CW is BECAUSE, that the face edges are CCW. If one draws it on paper
            // then it will become super clear.
            half_edge *iter = a.edge;
            const half_edge *end = iter;
            do { // we walk in CCW order around the vertex
                int cls1= classify_point(b, iter->origin->coords,
                                         iter->twin->origin->coords);
                auto* next = iter->twin->next;
                int cls2= classify_point(b, next->origin->coords,
                                         next->twin->origin->coords);
                // is (a,b) inside the cone so right of iter and left of next
                if(cls1<=0 && cls2>0)
                    return next;

                iter=iter->twin->next;
            } while(iter!=end);

            return iter;
        }

        template<typename number>
        auto planarize_division<number>::insert_edge_between_non_co_linear_vertices(half_edge *vertex_a_edge,
                                                                                    half_edge *vertex_b_edge,
                                                                                    dynamic_pool & pool) -> half_edge * {
            // insert edge between two vertices in a face, that are not co linear with one of the 4 walls, located
            // by their leaving edges. co linearity means the vertices lie on the same boundary ray.
            // for that case, we have a different procedure to handle.
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
            face->edge= e;
            face_2->edge= e_twin;
            // now, iterate on all of the conflicts of f, and move the correct ones to f2
            re_distribute_conflicts_of_split_face(e->face->conflict_list, e);
            // all edges of face_2 point to the old face_1, we need to change that
            walk_and_update_edges_face(e_twin, face_2);
            return e;
        }

        template<typename number>
        void planarize_division<number>::handle_co_linear_edge_with_trapeze(const trapeze_t &trapeze,
                                                                            const vertex & a,
                                                                            const vertex & b,
                                                                            const point_class_with_trapeze &wall_class, // left/top/right/bottom only
                                                                            half_edge ** result_edge_a,
                                                                            half_edge ** result_edge_b,
                                                                            dynamic_pool& pool) {
            // given that (a) and (b) are on the boundary and edge (a,b) is co-linear with one of the 4 boundaries of the trapeze,
            // try inserting vertices on that boundary, update windings along their path/between them and return the
            // corresponding start and end half edges whose start points are (a) and (b)
            auto * edge_vertex_a =
                    try_insert_vertex_on_trapeze_boundary_at(a, trapeze, wall_class, pool);
            auto * edge_vertex_b =
                    try_insert_vertex_on_trapeze_boundary_at(b, trapeze, wall_class, pool);
            // this is a dangerous practice, would be better to return a struct with pointers
            if(result_edge_a) *result_edge_a = edge_vertex_a;
            if(result_edge_b) *result_edge_b = edge_vertex_b;

            const int winding = infer_edge_winding(a, b);
            if(winding==0) return;

            auto *start = edge_vertex_a;
            auto *end = edge_vertex_b;
            // sort if needed
            bool before = is_a_before_or_equal_b_on_same_boundary(a, b, wall_class);
            if(!before) { auto *temp=start;start=end;end=temp; }
            while(start!=end) {
                start->winding+=winding;
                start->twin->winding=start->winding;
                start=start->next;
            }

        }

        template<typename number>
        auto planarize_division<number>::handle_vertical_face_cut(const trapeze_t &trapeze, vertex & a,
                                                                  const point_class_with_trapeze &a_classs,
                                                                  dynamic_pool & dynamic_pool) -> vertical_face_cut_result {
            // the procedure returns the half edge of the face for which it's origin vertex
            // is the insertion point. the half_edge_vertex *a will be inserted in place if it didn't exist yet.
            // the mission of this method is to split a little to create an intermediate face
            // for the regular procedure, this procedure might split a face to two or just
            // split an edge on which the vertex lies on.
            // todo: what about degenerate trapeze with three vertices ?
            // given that vertex (a) is in the closure of the trapeze
            // two things can occur, the vertex is completely inside the trapeze
            // 1. the vertex is strictly inside the horizontal part of the trapeze,
            //    therefore, add a vertical segment that cuts the top/bottom segments
            // 1.1. this also induces a splitting of the face into two faces including updating
            //      references and conflicts
            // 1.2. if the vertex was completely inside the trapeze(not on the top/bottom boundary),
            //      then split that vertical line as well. Note, that the vertical line is unique because
            //      of the strict horizontal inclusion.
            // 2. the vertex is on the left/right walls (including endpoints, i.e not necessarily strict inside),
            //    in this case just split the vertical left or right walls vertically by inserting a vertex if not exist already.
            const bool on_boundary = a_classs != point_class_with_trapeze::strictly_inside;
            const bool on_boundary_vertices = a_classs == point_class_with_trapeze::boundary_vertex;
            const bool in_left_wall = a_classs == point_class_with_trapeze::left_wall;
            const bool in_right_wall = a_classs == point_class_with_trapeze::right_wall;
            const bool in_top_wall = a_classs == point_class_with_trapeze::top_wall;
            const bool should_try_split_horizontal_trapeze_parts = !in_left_wall && !in_right_wall && !on_boundary_vertices;
            vertical_face_cut_result result{};
            result.left_trapeze = result.right_trapeze = trapeze;
            // vertical edge that a sits on
            half_edge * outgoing_vertex_edge=nullptr;
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
                // edge cannot exist yet because we are strictly inside horizontal part.
                // we insert a vertical edge, that starts at bottom edge into the top wall (bottom-to-top)
                auto *start_vertical_wall = insert_edge_between_non_co_linear_vertices(bottom_vertex_edge,
                                                                                       top_vertex_edge,dynamic_pool);
                // clamp vertex to this new edge endpoints if it is before or after
                // this fights the geometric numeric precision errors, that can happen in y coords
                clamp_vertex_horizontally(a, start_vertical_wall->origin->coords, start_vertical_wall->twin->origin->coords);
                clamp_vertex_vertically(a, start_vertical_wall->origin->coords, start_vertical_wall->twin->origin->coords);
                // update resulting trapezes
                result.left_trapeze.right_bottom = start_vertical_wall;
                result.left_trapeze.right_top = start_vertical_wall->next;
                result.right_trapeze.left_top = start_vertical_wall->twin;
                result.right_trapeze.left_bottom = start_vertical_wall->twin->next;
                // if the vertex is on the edge boundary, it will not split of course
                outgoing_vertex_edge = try_split_edge_at(a, start_vertical_wall, dynamic_pool);
            } // else we are on left or right walls already
            else // we are on left or right boundary
                outgoing_vertex_edge=try_insert_vertex_on_trapeze_boundary_at(a, trapeze, a_classs, dynamic_pool);

            result.face_was_split = should_try_split_horizontal_trapeze_parts;
            result.vertex_a_edge_split_edge = outgoing_vertex_edge;

            return result;
        }

        template<typename number>
        auto planarize_division<number>::handle_face_split(const trapeze_t & trapeze,
                                                           vertex &a, vertex &b,
                                                           const point_class_with_trapeze &a_class,
                                                           const point_class_with_trapeze &b_class,
                                                           half_edge ** result_edge_a,
                                                           half_edge ** result_edge_b,
                                                           dynamic_pool & dynamic_pool) -> bool {
            // given that edge (a,b) should split the face, i.e inside the face and is NOT
            // co-linear with a boundary wall of trapeze (for that case we have another procedure),
            // split the face to up to 4 pieces, strategy is:
            // 1. use a vertical that goes through "a" to try and split the trapeze
            // 2. if a cut has happened, two trapezes were created and "a" now lies on
            //    the vertical wall between them.
            // 3. infer the correct trapeze where "b" is in and perform roughly the same
            //    operation
            // 4. connect the two points, so they again split a face (in case they are not vertical)

            const int winding = infer_edge_winding(a, b);
            // first, in order to avoid robust issues later, we insert the (b) vertex on the trapeze.
            // if (b) is not strictly inside, then it is on the boundary of the trapeze, so let's
            // insert the vertex ASAP before the cuts begin
            const bool b_is_on_original_boundary = b_class!=point_class_with_trapeze::strictly_inside;
            if(b_is_on_original_boundary)
                try_insert_vertex_on_trapeze_boundary_at(b, trapeze, b_class, dynamic_pool);
            // first, try to split vertically with endpoint (a)
            vertical_face_cut_result a_cut_result = handle_vertical_face_cut(trapeze,
                                                                             a, a_class, dynamic_pool);
            // let's find out where (b) ended up after a split (that might have not happened)
            const trapeze_t * trapeze_of_b = &trapeze;
            point_class_with_trapeze b_new_class = b_class;
            if(a_cut_result.face_was_split) {
                // a vertical split happened, let's see where b is
                bool left = b.x<=a_cut_result.vertex_a_edge_split_edge->origin->coords.x;
                trapeze_of_b = left ? &a_cut_result.left_trapeze : &a_cut_result.right_trapeze;
                // now, we need to update vertex (b) class after the vertical (a) cut in a robust manner.
                // - If b was on boundary, then it is still on a boundary but might change to a boundary_vertex class
                //   this happens if a.x==b.x
                // - if b had 'strictly_inside' class, then it stays the same unless a.x==b.x and then it
                //   changes class into left/right wall
                {
                    if(a.x==b.x) {
                        if(b_is_on_original_boundary)
                            b_new_class= point_class_with_trapeze::boundary_vertex;
                        else { // b was strictly inside in original trapeze
                            b_new_class = trapeze_of_b->left_bottom->origin->coords.x == b.x ?
                                          point_class_with_trapeze::left_wall :
                                          point_class_with_trapeze::right_wall;
                        }
                    }
                }
            }

            // next, try to split trapeze_of_b vertically with b endpoint
            vertical_face_cut_result b_cut_result = handle_vertical_face_cut(*trapeze_of_b,
                                                                             b, b_new_class, dynamic_pool);
            const trapeze_t *mutual_trapeze = trapeze_of_b;
            // now, infer the mutual trapeze of a and b after the vertical splits have occurred
            if(b_cut_result.face_was_split) {
                // a vertical split happened, let's see where b is
                bool left = a.x<=b_cut_result.vertex_a_edge_split_edge->origin->coords.x;
                mutual_trapeze = left ? &b_cut_result.left_trapeze : &b_cut_result.right_trapeze;
                bool deg = a.x==b.x;
//                if(deg) {
//                    if(b_cut_result.left_trapeze.left_top->origin->coords.x==b_cut_result.left_trapeze.right_top->origin->coords.x)
//                        mutual_trapeze = &b_cut_result.left_trapeze;
//                    else
//                        mutual_trapeze = &b_cut_result.right_trapeze;
//                }
            }

            // two options ?
            // 1. (a,b) is vertical, need to only update it's winding
            // 2. (a,b) is NOT vertical, connect a new edge between them, split
            // 3. BUT, first we need to infer the correct half edges, that go out of a and b
            //    and have the mutual face to their left

            // any edge of the trapeze of b will tell us the face
            const auto * face = mutual_trapeze->right_bottom->face;
            // a_edge/b_edge both are half edges rooted at a/b respectively and such that both belong to same face
            auto * a_edge = locate_half_edge_of_face_rooted_at_vertex(a_cut_result.vertex_a_edge_split_edge->origin, face);
            auto * b_edge = locate_half_edge_of_face_rooted_at_vertex(b_cut_result.vertex_a_edge_split_edge->origin, face);
            bool no_split = a_edge==b_edge; // can happen due to numeric errors
//            if(no_split) {
//                *result_edge_a = *result_edge_b = a_edge;
//                return false;
//            }
            auto * inserted_edge = a_edge;
            bool is_a_b_vertical = a_edge->origin->coords.x==b_edge->origin->coords.x;
            if(!is_a_b_vertical)
                inserted_edge = insert_edge_between_non_co_linear_vertices(a_edge, b_edge,
                                                                           dynamic_pool);

            inserted_edge->winding += winding;
            inserted_edge->twin->winding = inserted_edge->winding; // a_edge->twin is b_edge

            *result_edge_a = inserted_edge;
            *result_edge_b = inserted_edge->twin;
            return true;
        }

        template<typename number>
        auto planarize_division<number>::handle_face_merge(const half_edge_vertex *vertex_on_vertical_wall
        ) -> void {
            // given that vertex v is on a past vertical wall (left/right) boundary, we want to check 2 things:
            // 1. the vertical top/bottom edge that adjacent to it, is single(not divided) in it's adjacent trapezes
            // 2. has winding==0
            // 3. horizontal (top/bottom) wall that touches it from_sampler both side is almost a line
            // if all the above happens then we can remove that edge and maintain a variant of the contraction principle.
            // now, try to test if we can shrink the top or bottom or both edges adjacent to the vertex
            // how we do it: examine upwards/downwards half edge
            // 1. if it has winding!=0 skip
            // 2. if there is another edge after it on the vertical wall, skip as well
            // 3. examine how close the horizontal top/bottom edges that lie at the end
            // 4. if they are close than an epsilon, remove the vertical edge
            // 4.1. concatenate the conflicting edges list
            // 4.2. walk on the right face's edges and re-point all half edges to the left face
            const half_edge_vertex *v = vertex_on_vertical_wall;
            half_edge * candidates[2] = {nullptr, nullptr};
            int index=0;
            half_edge *iter = v->edge;
            half_edge *top_edge=nullptr, *bottom_edge=nullptr;
            const half_edge *end = iter;
            do { // we walk in CCW order around the vertex
                bool is_vertical_edge = iter->twin->origin->coords.x==v->coords.x;
                if(is_vertical_edge) {
                    bool is_top = iter->twin->origin->coords.y < v->coords.y; // equality cannot happen
                    if(is_top)
                        top_edge= iter;
                    else
                        bottom_edge= iter;

                    candidates[index++]= iter;
                }

                if(index==2)
                    break;

                iter=iter->twin->next;
            } while(iter!=end);

            // there have to be top and bottom because v was strictly inside the wall
            if(top_edge== nullptr || bottom_edge== nullptr) {
#if DEBUG_PLANAR==true
                throw std::runtime_error("handle_face_merge::have not found bottom or top edge !!!");
#endif
                return;
            }

            // iterate over up to two candidates
            for (int ix = 0; ix < index; ++ix) {
                auto * candidate_edge = candidates[ix];
                if(!candidate_edge)
                    break;

                // start with top edge
                // perform test 1 & 2
                bool valid = candidate_edge->winding==0 &&
                             (candidate_edge->next->twin->origin->coords.x!=candidate_edge->origin->coords.x);
                if(valid) {
                    // top edge goes up and CCW in it's face
                    // a___b___c
                    //     |
                    // ----v----
                    //     |
                    // c'__b'__a'
                    // now test how much is a-b-c looks like a line
                    // we do it by calculating the distance from_sampler vertex c to line (a,b)
                    // the illustration above is for top_edge= v-->b
                    const auto & a = candidate_edge->next->twin->origin->coords;
                    const auto & b = candidate_edge->twin->origin->coords;
                    const auto & c = candidate_edge->twin->prev->origin->coords;
                    // perform test #3
                    bool is_abc_almost_a_line = is_distance_to_line_less_than_epsilon(c, a, b, number(1));
                    if(is_abc_almost_a_line) {
                        // if it is almost a line, then (v,b) edge eligible for removal
                        remove_edge(candidate_edge);
                    }

                }

            }

        }

        template<typename number>
        bool planarize_division<number>::is_distance_to_line_less_than_epsilon(const vertex &v,
                                                                               const vertex &a, const vertex &b, number epsilon) {
            // we use the equation 2*A = h*d(a,b)
            // where A = area of triangle spanned by (a,b,v), h= distance of v to (a,b)
            // we raise everything to quads to avoid square function and we avoid division.
            // while this is more robust, you have to make sure that your number type will
            // not overflow (use 64 bit for fixed point integers)
            number numerator= abs((b.x - a.x) * (v.y - a.y) - (v.x - a.x) * (b.y - a.y)); // 2*A
            number numerator_quad = numerator*numerator; // (2A)^2
            number ab_length_quad = (b.y - a.y)*(b.y - a.y) + (b.x - a.x)*(b.x - a.x); // (length(a,b))^2
            number epsilon_quad = epsilon*epsilon;
            return numerator_quad < epsilon_quad*ab_length_quad;
        }

        template<typename number>
        void planarize_division<number>::remove_edge(half_edge *edge) {
            // remove an edge and it's twin, then:
            // 1.  re-connect adjacent edges
            // 2.  merge faces
            // 3.  move face_2's conflict list into face_1
            auto * face_1 = edge->face;
            auto * face_2 = edge->twin->face;
            // update right adjacent face edges' to point left face
            walk_and_update_edges_face(edge->twin, edge->face);
            // re-connect start segment
            edge->prev->next = edge->twin->next;
            edge->twin->next->prev = edge->prev;
            // re-connect end segment
            edge->twin->prev->next = edge->next;
            edge->next->prev= edge->twin->prev;
            // update the vertices edge links
            edge->origin->edge= edge->twin->next;
            edge->twin->origin->edge= edge->next;
            // move face_2's conflict list into face_1
            auto * conflict_ref= face_1->conflict_list;
            // move the pointer to the last conflict of face_1
            if(conflict_ref) {
                while (conflict_ref->next!=nullptr)
                    conflict_ref=conflict_ref->next;
                // hook the last conflict of face_1 to the first conflict of face_2
                conflict_ref->next = face_2->conflict_list;
            } else {
                face_1->conflict_list=conflict_ref = face_2->conflict_list;
            }
            // now update the conflicting edges with the correct face
            while (conflict_ref!=nullptr) {
                conflict_ref->edge->conflict_face=face_1;
                conflict_ref=conflict_ref->next;
            }
            face_2->edge=nullptr; face_2->conflict_list= nullptr;
        }

        template<typename number>
        void planarize_division<number>::insert_edge(half_edge *edge, index idx, dynamic_pool & dynamic_pool) {
            // start with the conflicting face and iterate carefully, a---b'---b
            bool are_we_done = false;
            int count = 0;
            vertex a, b, b_tag;
            half_edge_face * face = edge->conflict_face;
            point_class_with_trapeze wall_result;
            trapeze_t trapeze=infer_trapeze(face);
            point_class_with_trapeze class_a= round_vertex_to_trapeze(edge->origin->coords, trapeze);
            a=edge->origin->coords;
            b=edge->twin->origin->coords;
            if(a==b) return;

            while(!are_we_done) {
                half_edge *a_vertex_edge= nullptr, *b_tag_vertex_edge= nullptr;
                // the reporting of and class of the next interesting point of the edge against current trapeze
                conflicting_edge_intersection_status edge_status =
                        compute_conflicting_edge_intersection_against_trapeze(trapeze, a, b, class_a);
                b_tag = edge_status.point_of_interest;
                point_class_with_trapeze class_b_tag = edge_status.class_of_interest_point;

#if DEBUG_PLANAR==true
//                if(b_tag==a)
//                throw std::runtime_error("insert_edge():: a==b_tag, which indicates a problem !!!");
#endif
                // does edge (a,b') is co linear with boundary ? if so treat it
                bool co_linear_with_boundary = do_a_b_lies_on_same_trapeze_wall(trapeze, a, b_tag,
                                                                                class_a, class_b_tag,
                                                                                wall_result);
                if(co_linear_with_boundary) {
                    // co-linear, so let's just insert vertices on the boundary and handle windings
                    // we do not need to split face in this case
                    // Explain:: why don't we send vertical splits lines for boundary ? I think
                    // because the input is always a closed polyline so it doesn't matter
                    handle_co_linear_edge_with_trapeze(trapeze, a, b_tag, wall_result,
                                                       &a_vertex_edge, &b_tag_vertex_edge, dynamic_pool);
                } else {
                    // not co-linear so we have to split the trapeze into up to 4 faces
                    // btw, we at-least split vertically into two top and bottom pieces
                    const bool has_inserted_edge= handle_face_split(trapeze, a, b_tag, class_a, class_b_tag,
                                                     &a_vertex_edge, &b_tag_vertex_edge, dynamic_pool);

                    if(count==0) {
                        // handle_face_split ,method might change/clamp 'a' vertex. If this is
                        // the first endpoint, we would like to copy this change to source.
                        // this should contribute to robustness
                        edge->origin->coords=a_vertex_edge->origin->coords;
                    }
                }

                // now, we need to merge faces if we split a vertical wall, i.e, if
                // the new 'a' coord strictly lies in the left/right wall
                // record last split vertex if it was on a vertical wall and not the first vertex
                // and not a co-linear segment on the boundary
                bool candidate_merge = (count>=1) && !co_linear_with_boundary &&
                                       (class_a==point_class_with_trapeze::left_wall ||
                                        class_a==point_class_with_trapeze::right_wall);

                if(candidate_merge&&(APPLY_MERGE))
                    handle_face_merge(a_vertex_edge->origin);

                // increment
                // if b'==b we are done
                are_we_done = b==b_tag || count>200;
                if(are_we_done)
                    break;
                // todo: if b is the last endpoint of the edge, update it's coordinates with the original
                // todo: source copy, in case there was a clamping

                // ITERATION STEP, therefore update:
                // 1. now iterate on the sub-line (b', b), by assigning a=b'
                // 2. locate the face, that (b', b) is intruding into / conflicting
                // 3. infer the trapeze of the face
                // 4. infer the class of the updated vertex a=b' in this face
                a =b_tag;
                half_edge * located_face_edge= locate_face_of_a_b(*b_tag_vertex_edge->origin, b);
                face = located_face_edge->face;
                trapeze=infer_trapeze(face);
                class_a = locate_and_classify_point_that_is_already_on_trapeze(a, trapeze);
                count++;
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
                insert_edge(e, ix, dynamic_pool);
            }

        }

        template<typename number>
        void planarize_division<number>::compute_DEBUG(const chunker<vertex> &pieces,
                                                       dynamic_array<vertex> &debug_trapezes) {

            // vertices size is also edges size since these are polygons
            const auto v_size = pieces.unchunked_size();
            // plus 4s for the sake of frame
            static_pool static_pool(v_size + 4, 4*2 + v_size*2, 1, v_size);
            dynamic_pool dynamic_pool{};
            // create the main frame
            auto * main_face = create_frame(pieces, static_pool);
            // create edges and conflict lists
            auto ** edges_list = build_edges_and_conflicts(pieces, *main_face, static_pool);
            // todo: here create a random permutation of edges

            // now start iterations
            for (int ix = 0; ix < v_size; ++ix) {
//        for (int ix = 0; ix < 6; ++ix) {
                auto * e = edges_list[ix];

                //remove_edge_from_conflict_list(e);
                insert_edge(e, ix, dynamic_pool);
            }

            // collect trapezes so far
            face_to_trapeze_vertices(main_face, debug_trapezes);
            auto &faces = dynamic_pool.getFaces();
            int count_active_faces= 0;
            for (index ix = 0; ix < faces.size(); ++ix) {
                if(faces[ix]->edge!= nullptr)
                    count_active_faces++;
                face_to_trapeze_vertices(faces[ix], debug_trapezes);
            }

            std::cout<< "# active faces: " << count_active_faces <<std::endl;
        }

        template<typename number>
        void planarize_division<number>::clamp(number &val, const number & a, const number &b) {
            if(val<a) val = a;
            if(val>b) val = b;
        }

        template<typename number>
        void planarize_division<number>::clamp_vertex(vertex &v, vertex a, vertex b) {
            bool is_a_before_b = a.x<b.x || (a.x==b.x && a.y<b.y);
            if(!is_a_before_b) { vertex c=a;a=b;b=c; } // sort, so a is before b
            bool is_v_before_a = v.x<a.x || (v.x==a.x && v.y<a.y);
            if(is_v_before_a) v=a;
            bool is_v_after_b = v.x>b.x || (v.x==b.x && v.y>=b.y);
            if(is_v_after_b) v=b;
        }

        template<typename number>
        void planarize_division<number>::clamp_vertex_horizontally(vertex &v, vertex a, vertex b) {
            if(a.x>b.x) { vertex c=a;a=b;b=c; }
            if(v.x<a.x) v=a;
            if(v.x>b.x) v=b;
        }

        template<typename number>
        void planarize_division<number>::clamp_vertex_vertically(vertex &v, vertex a, vertex b) {
            if(a.y>b.y) { vertex c=a;a=b;b=c; }
            if(v.y<a.y) v=a;
            if(v.y>b.y) v=b;
        }

        template<typename number>
        bool planarize_division<number>::is_trapeze_degenerate(const trapeze_t & trapeze) {
            return (trapeze.left_top->origin==trapeze.left_bottom->origin) ||
                   (trapeze.right_top->origin==trapeze.right_bottom->origin);
        }

        template<typename number>
        int planarize_division<number>::infer_edge_winding(const vertex & a, const vertex & b) {
            // infer winding of edge (a,b)
            if(b.y<a.y) return 1; // rising/ascending edge
            if(b.y>a.y) return -1; // descending edge
            return 0;
        }

        template<typename number>
        bool planarize_division<number>::do_a_b_lies_on_same_trapeze_wall(const trapeze_t & trapeze,
                                                                          const vertex &a,
                                                                          const vertex &b,
                                                                          const point_class_with_trapeze & a_class,
                                                                          const point_class_with_trapeze & b_class,
                                                                          point_class_with_trapeze &resulting_wall) {
            // given an edge (a,b) where both a and b belong to trapeze, test if the edge is co-linear with one of the
            // boundaries of the trapeze. we classify symbolically rather than analytically in order to be robust.
            // this procedure also handles degenerate trapezes. if true, then resulting_wall will indicate which wall
            // can be one of four: left_wall, right_wall, top_wall, bottom_wall
            // skip inside
            if(a_class==point_class_with_trapeze::strictly_inside ||
               b_class==point_class_with_trapeze::strictly_inside)
                return false;

            bool a_on_boundary_vertex = a_class==point_class_with_trapeze::boundary_vertex;
            bool b_on_boundary_vertex = b_class==point_class_with_trapeze::boundary_vertex;
            bool same_class = a_class==b_class;

            // test if on same wall
            if(same_class && !a_on_boundary_vertex && !b_on_boundary_vertex) {
                resulting_wall=a_class;
                return true;
            }
            // todo: what happens when trapeze is degenerate - a triangle
            bool a_on_wall, b_on_wall;
            // test left wall
            a_on_wall = a_class==point_class_with_trapeze::left_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.left_top->origin->coords || a==trapeze.left_bottom->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::left_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.left_top->origin->coords || b==trapeze.left_bottom->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::left_wall;
                return true;
            }

            // test right wall
            a_on_wall = a_class==point_class_with_trapeze::right_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.right_top->origin->coords || a==trapeze.right_bottom->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::right_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.right_top->origin->coords || b==trapeze.right_bottom->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::right_wall;
                return true;
            }

            // test top wall
            a_on_wall = a_class==point_class_with_trapeze::top_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.right_top->origin->coords || a==trapeze.left_top->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::top_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.right_top->origin->coords || b==trapeze.left_top->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::top_wall;
                return true;
            }

            // test bottom wall
            a_on_wall = a_class==point_class_with_trapeze::bottom_wall ||
                        (a_on_boundary_vertex &&
                         (a==trapeze.left_bottom->origin->coords || a==trapeze.right_bottom->origin->coords));
            b_on_wall = b_class==point_class_with_trapeze::bottom_wall ||
                        (b_on_boundary_vertex &&
                         (b==trapeze.left_bottom->origin->coords || b==trapeze.right_bottom->origin->coords));
            if(a_on_wall && b_on_wall) {
                resulting_wall=point_class_with_trapeze::bottom_wall;
                return true;
            }

            return false;
        }

        template<typename number>
        auto planarize_division<number>::compute_conflicting_edge_intersection_against_trapeze(const trapeze_t & trapeze,
                                                                                               vertex &a, vertex b,
                                                                                               const point_class_with_trapeze & a_class)
        -> conflicting_edge_intersection_status {
            // given that edge (a,b), vertex (a) is conflicting, i.e on boundary or completely inside
            // and we know that the edge passes through the trapeze or lies on the boundary,
            // find the second interesting point, intersection or overlap or completely inside
            // NOTES:
            // ** About (b) vertex
            // 1. because of precision errors, (b) might be completely outside.
            // 2. therefore, if we know the wall of (a) and suppose (b) is right of that wall,
            //    then we need to clamp/round the vertex class
            //
            // ** About (a) vertex
            // 0. vertex (a) is classified to be in the closure of trapeze (even if analytically it might show otherwise)
            // 1. first, if (b) is strictly in or on trapeze boundary, then we are DONE!!
            // 2. although endpoint "a" belongs to the trapeze, there is a chance it might be outside geometrically,
            //    because of numeric precision errors, that occur during vertical splits
            // 3. therefore, we might get two proper intersections for (a,b) edge against
            //    top/bottom/left/right boundaries.
            // 4. therefore, always consider the intersection with the biggest alpha as what we want.
            //

            point_class_with_trapeze cla_b= round_edge_to_trapeze(a, b, a_class, trapeze);
            conflicting_edge_intersection_status result{};
            result.class_of_interest_point = cla_b;
            result.point_of_interest = b;
            // if vertex (b) is inside or on the boundary, we are done
            bool b_in_exterior = cla_b==point_class_with_trapeze::outside;
            bool b_in_interior = cla_b==point_class_with_trapeze::strictly_inside;
            bool b_in_closure = !b_in_exterior;
            bool b_in_boundary = !b_in_interior && !b_in_exterior;
            if(b_in_closure)
                return result;

            // if we got here, vertex (a) is in the closure and vertex (b) is outside/exterior.
            // the strategy:
            // - try to round now the edge from_sampler interior to a wall boundary is they pass a certain threshold
            // - compute intersections against walls and pick the correct one
            // first test if we have b cases
            bool a_is_on_boundary= a_class!=point_class_with_trapeze::strictly_inside;

            // first test if we have a cases
            // we now know, that b is outside, therefore hunt proper intersections
            vertex intersection_point{};
            number alpha(0), alpha1(0);
//            number biggest_alpha=number(0);
            intersection_status status;
//            bool is_bigger_alpha;

            { // left-wall
                const auto &start = trapeze.left_top->origin->coords;
                const auto &end = trapeze.left_bottom->origin->coords;
                bool a_on_wall = a_class==point_class_with_trapeze::left_wall || (a==start || a==end);
                if(!a_on_wall) {
                    status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                    if(status==intersection_status::intersect) {
//                        is_bigger_alpha = alpha>=biggest_alpha;
//                        if(is_bigger_alpha) {
//                            biggest_alpha=alpha;
                            result.class_of_interest_point = point_class_with_trapeze::left_wall;
                            result.point_of_interest = intersection_point;
                            result.point_of_interest.x=start.x;
                            clamp(result.point_of_interest.y, start.y, end.y); // important to clamp for symbolic reasons as well in case of numeric errors.
                            if(result.point_of_interest==start || result.point_of_interest==end)
                                result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        }
                    }

                }

//                else if(status==intersection_status::parallel) {
//                    const bool a_on_wall= a.x==start.x;
//                    if(a_on_wall) {
//                        result.class_of_interest_point = point_class_with_trapeze::left_wall;
//                        result.point_of_interest = b;
//                        result.point_of_interest.x=start.x;
//                        clamp(result.point_of_interest.y, start.y, end.y); // important to clamp for symbolic reasons as well in case of numeric errors.
//                        if(result.point_of_interest==start || result.point_of_interest==end)
//                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        return result;
//                    }
//                }

            }
            { // right wall
                const auto &start = trapeze.right_bottom->origin->coords;
                const auto &end = trapeze.right_top->origin->coords;
                bool a_on_wall = a_class==point_class_with_trapeze::right_wall || (a==start || a==end);
                if(!a_on_wall) {
                    status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                    if(status==intersection_status::intersect) {
//                        is_bigger_alpha = alpha>=biggest_alpha;
//                        if(is_bigger_alpha) {
//                            biggest_alpha = alpha;
                            result.class_of_interest_point = point_class_with_trapeze::right_wall;
                            result.point_of_interest = intersection_point;
                            result.point_of_interest.x = start.x;
                            clamp(result.point_of_interest.y, end.y, start.y);
                            if (result.point_of_interest == start || result.point_of_interest == end)
                                result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        }
                    }
                }
//                else if(status==intersection_status::parallel) {
//                    const bool a_on_wall= a.x==start.x;
//                    if(a_on_wall) {
//                        result.class_of_interest_point = point_class_with_trapeze::right_wall;
//                        result.point_of_interest = b;
//                        result.point_of_interest.x=start.x;
//                        clamp(result.point_of_interest.y, end.y, start.y);
//                        if(result.point_of_interest==start || result.point_of_interest==end)
//                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        return result;
//                    }
//
//                }

            }
            { // bottom wall
                const auto &start = trapeze.left_bottom->origin->coords;
                const auto &end = trapeze.right_bottom->origin->coords;
                const bool a_on_wall= a==start || a==end || a_class==point_class_with_trapeze::bottom_wall;
                if(!a_on_wall) {
                    status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                    if(status==intersection_status::intersect) {
//                        is_bigger_alpha = alpha>=biggest_alpha;
//                        if(is_bigger_alpha) {
//                            biggest_alpha=alpha;
                            result.class_of_interest_point = point_class_with_trapeze::bottom_wall;
                            result.point_of_interest = intersection_point;
                            if(result.point_of_interest==start || result.point_of_interest==end)
                                result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        }
                    }

                }
//                else if(status==intersection_status::parallel) {
//                    const bool a_on_wall= a==start || a==end || a_class==point_class_with_trapeze::bottom_wall;
//                    if(a_on_wall) {
//                        result.class_of_interest_point = point_class_with_trapeze::bottom_wall;
//                        result.point_of_interest = b;
//                        clamp_vertex_horizontally(result.point_of_interest, start, end);
//                        if(result.point_of_interest==start || result.point_of_interest==end)
//                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        return result;
//                    }
//
//                }

            }
            { // top wall
                const auto &start = trapeze.right_top->origin->coords;
                const auto &end = trapeze.left_top->origin->coords;
                const bool a_on_wall= a==start || a==end || a_class==point_class_with_trapeze::top_wall;
                if(!a_on_wall) {
                    status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                    if(status==intersection_status::intersect) {
//                        is_bigger_alpha = alpha>=biggest_alpha;
//                        if(is_bigger_alpha) {
//                            biggest_alpha=alpha;
                            result.class_of_interest_point = point_class_with_trapeze::top_wall;
                            result.point_of_interest = intersection_point;
                            if(result.point_of_interest==start || result.point_of_interest==end)
                                result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        }

                    }
                }
//                else if(status==intersection_status::parallel) {
//                    const bool a_on_wall= a==start || a==end || a_class==point_class_with_trapeze::top_wall;
//                    if(a_on_wall) {
//                        result.class_of_interest_point = point_class_with_trapeze::top_wall;
//                        result.point_of_interest = b;
//                        clamp_vertex_horizontally(result.point_of_interest, start, end);
//                        if(result.point_of_interest==start || result.point_of_interest==end)
//                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
//                        return result;
//                    }
//                }

            }

            return result;
        }

        template<typename number>
        auto planarize_division<number>::finite_segment_intersection_test(const vertex &a, const vertex &b,
                                                                          const vertex &c, const vertex &d,
                                                                          vertex & intersection,
                                                                          number &alpha, number &alpha1) -> intersection_status{
            // this procedure will find proper and improper(touches) intersections, but no
            // overlaps, since overlaps induce parallel classification, this would have to be resolved outside
            if(a==b || c==d)
                return intersection_status::degenerate_line;
            auto ab = b - a;
            auto cd = d - c;
            auto dem = ab.x * cd.y - ab.y * cd.x;

            // parallel lines
            if (abs(dem) <= number(1))
                return intersection_status::parallel;
            else {
                auto ca = a - c;
                auto numerator_1 = ca.y*cd.x - ca.x*cd.y;
                auto numerator_2 = -ab.y*ca.x + ab.x*ca.y;

                if (dem > 0) {
                    if (numerator_1 < 0 || numerator_1 > dem ||
                        numerator_2 < 0 || numerator_2 > dem)
                        return intersection_status::none;
                } else {
                    if (numerator_1 > 0 || numerator_1 < dem ||
                        numerator_2 > 0 || numerator_2 < dem)
                        return intersection_status::none;
                }

                // avoid division on trivial edge cases
                if(numerator_1==0) { // a lies on c--d segment
                    alpha=0; intersection = a;
                }
                else if(numerator_1==dem) { // b lies on c--d segment
                    alpha=1; intersection = b;
                }
                else if(numerator_2==0) { // c lies on a--b segment
                    alpha=0; intersection = c;
                }
                else if(numerator_2==dem) { // d lies on a--b segment
                    alpha=1; intersection = d;
                }
                else { // proper intersection
                    alpha = numerator_1/dem;
                    // alpha1 = numerator_2/dem;
//                    intersection = a + ab*alpha;
                    intersection = a + (ab*numerator_1)/dem;
                }
            }

            return intersection_status::intersect;
        }

        template <typename number>
        inline int
        planarize_division<number>::classify_point(const vertex & point, const vertex &a, const vertex & b) {
            // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
            // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))
            //    Input:  three points p, a, b
            //    Return: >0 for P left of the line through a and b
            //            =0 for P  on the line
            //            <0 for P  right of the line
            //    See: Algorithm 1 "Area of Triangles and Polygons"
            auto result= (b.x-a.x)*(point.y-a.y)-(point.x-a.x)*(b.y-a.y);
            if(result<0) return 1;
            else if(result>0) return -1;
            else return 0;
        }

    }
}

```