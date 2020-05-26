namespace microgl {

    namespace tessellation {

        template<typename number>
        auto planarize_division<number>::create_frame(const chunker<vertex> &pieces,
                                                      dynamic_pool & dynamic_pool) -> half_edge_face * {
            const auto pieces_length = pieces.size();
            vertex left_top=pieces.raw_data()[0];
            vertex right_bottom=left_top;
            // find bbox of all
            for (index ix = 0; ix < pieces_length; ++ix) {
                auto const piece = pieces[ix];
                const auto piece_size = piece.size;
                if(piece_size<=1) continue;
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
            auto * v0 = dynamic_pool.create_vertex(left_top);
            auto * v1 = dynamic_pool.create_vertex({left_top.x, right_bottom.y});
            auto * v2 = dynamic_pool.create_vertex(right_bottom);
            auto * v3 = dynamic_pool.create_vertex({right_bottom.x, left_top.y});
            // half edges of vertices
            auto * edge_0 = dynamic_pool.create_edge();
            auto * edge_1 = dynamic_pool.create_edge();
            auto * edge_2 = dynamic_pool.create_edge();
            auto * edge_3 = dynamic_pool.create_edge();
            // twins of edges, todo:: i might not need twins for the frame
            auto * edge_0_twin = dynamic_pool.create_edge();
            auto * edge_1_twin = dynamic_pool.create_edge();
            auto * edge_2_twin = dynamic_pool.create_edge();
            auto * edge_3_twin = dynamic_pool.create_edge();
            // connect edges to vertices, CCW from left-top vertex
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
            auto * face = dynamic_pool.create_face();
            edge_0->face = face; edge_0_twin->face = nullptr; // nullptr for face is the outside world face marker
            edge_1->face = face; edge_1_twin->face = nullptr;
            edge_2->face = face; edge_2_twin->face = nullptr;
            edge_3->face = face; edge_3_twin->face = nullptr;
            // CCW around face, face is always to the left of the walk
            face->edge = edge_0;
            return face;
        }

        template<typename number>
        auto planarize_division<number>::build_poly_and_conflicts(const chunker<vertex> &pieces,
                                                                  half_edge_face & main_frame,
                                                                  poly_info ** poly_list_out,
                                                                  conflict ** conflict_list_out) -> void {
            // given polygons, build their vertices, edges and conflicts
            const auto pieces_length = pieces.size();
            conflict * conflict_first = nullptr;
            auto * poly_list = new poly_info[pieces_length];
            auto * conflict_list = new conflict[pieces_length];

            for (index ix = 0; ix < pieces_length; ++ix) {
                auto piece = pieces[ix];
                const auto piece_size = piece.size;
                auto & poly= poly_list[ix];
                poly.id=ix;
                poly.is_closed=true;
                poly.size=0;
                poly.points=nullptr;
                poly.conflict_face= nullptr;
                if(piece_size<=1) continue;
                index last_index= piece_size-1, first_index=0;
                while((first_index+1<piece_size)&& piece.data[first_index]==piece.data[first_index+1])
                    first_index++;
                while(last_index>0 && piece.data[last_index]==piece.data[first_index])
                    last_index--;
                const index new_size=last_index-first_index+1;
                if(new_size<=1) continue;
                poly.size=new_size;
                poly.points=&piece.data[first_index];
                poly.conflict_face= &main_frame;
                auto * c = &conflict_list[ix];
                c->unadded_input_poly=&poly;
                if(conflict_first) c->next = conflict_first;
                conflict_first = c;
            }

            main_frame.conflict_list = conflict_first;

            *poly_list_out=poly_list;
            *conflict_list_out=conflict_list;
        }

        template<typename number>
        auto planarize_division<number>::infer_trapeze(const half_edge_face *face) -> trapeze_t {
            if(face==nullptr || face->edge==nullptr)
                throw_regular(string_debug("infer_trapeze()::trying to infer a trapeze of a probably merged/deleted face !!!"));
            auto * e = face->edge;
            const auto * e_end = face->edge;
            trapeze_t trapeze;
            trapeze.left_top = trapeze.left_bottom = trapeze.right_bottom = trapeze.right_top = e;
            do { // we are walking CCW
                const auto * v = e->origin;
                auto curr_x = v->coords.x, curr_y = v->coords.y;
                if(curr_x < trapeze.left_top->origin->coords.x || (curr_x==trapeze.left_top->origin->coords.x && curr_y < trapeze.left_top->origin->coords.y))
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
        auto planarize_division<number>::round_vertex_to_trapeze(vertex & point, const trapeze_t &trapeze) -> point_class_with_trapeze {
            // given that point that should belong to trapeze, BUT may not sit properly or outside because
            // of precision errors, geometrically round it into place
            location_codes codes;
            codes.compute_location_codes(point, trapeze);
            if(codes.isInClosure()) {
                if(codes.isBoundaryVertex()) return point_class_with_trapeze::boundary_vertex;
                if(codes.isInterior()) return point_class_with_trapeze::strictly_inside;
            }
            codes.fill();
            for (int ix = 0; ix < 4; ++ix) {
                bool suspect = codes.array[ix]<=0;
                if(suspect) {
                    const auto wall= codes.classOfIndex(ix);
                    vertex start, end;
                    wall_vertex_endpoints(trapeze, wall, start, end);
                    point.x=clamp(point.x, start.x, end.x);
                    bool is_horizontal_wall= start.x!=end.x;
                    if(is_horizontal_wall)
                        point.y= evaluate_line_at_x(point.x, start, end);
                    point.y=clamp(point.y, start.y, end.y);
                    point=clamp_vertex_to_trapeze_wall(point, wall, trapeze);
                    if(point==start || point==end)
                        return point_class_with_trapeze::boundary_vertex;
                    return wall;
                }
            }
        }

        template<typename number>
        auto planarize_division<number>::clamp_vertex_to_trapeze_wall(const vertex & v,
                                                                      const point_class_with_trapeze & wall,
                                                                      const trapeze_t &trapeze) -> vertex {
            vertex start, end, result=v;
            wall_vertex_endpoints(trapeze, wall, start, end);
            result.x=clamp(result.x, start.x, end.x);
            result.y=clamp(result.y, start.y, end.y);
            if(start.x!=end.x) { // very important for horizontal edges
                if(result.x==start.x) result=start;
                else if(result.x==end.x) result=end;
            }
            return result;
        }

        template<typename number>
        auto planarize_division<number>::locate_and_classify_vertex_that_is_already_on_trapeze(const half_edge_vertex * v,
                                                                                              const trapeze_t &trapeze)
                -> vertex_location_result {
            // given that the planar vertex IS present on the trapeze boundary
            if(v==trapeze.left_top->origin) return {point_class_with_trapeze::boundary_vertex, trapeze.left_top};
            if(v==trapeze.left_bottom->origin) return {point_class_with_trapeze::boundary_vertex, trapeze.left_bottom};
            if(v==trapeze.right_bottom->origin) return {point_class_with_trapeze::boundary_vertex, trapeze.right_bottom};
            if(v==trapeze.right_top->origin) return {point_class_with_trapeze::boundary_vertex, trapeze.right_top};
            for (int ix = 0; ix < 4; ++ix) {
                const auto wall=location_codes::classOfIndex(ix);
                half_edge *start= nullptr, *end= nullptr;
                wall_edge_endpoints(trapeze, wall, &start, &end);
                auto * iter = start;
                while(iter!=end) {
                    if(v==iter->origin) return {wall, iter};
                    iter=iter->next;
                }
            }
            return {point_class_with_trapeze::unknown, nullptr};
        }

        template<typename number>
        auto planarize_division<number>::try_split_edge_at(const vertex& point,
                                                           half_edge *edge,
                                                           dynamic_pool & pool) -> half_edge * {
            // let's shorten both edge and it's twin,each from it's side
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
                    throw_regular(string_debug("try_insert_vertex_on_trapeze_boundary_at:: wall class is not boundary !!!"));
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
            throw_regular(string_debug("try_insert_vertex_on_trapeze_boundary_at:: didnt find where !!!"));
            return nullptr;
        }

        template<typename number>
        number planarize_division<number>::evaluate_line_at_x(const number x, const vertex &a, const vertex &b) {
            if(x==a.x) return a.y; if(x==b.x) return b.y;
            return a.y + (((b.y-a.y)*(x-a.x))/(b.x-a.x));
        }

        template<typename number>
        void planarize_division<number>::re_distribute_conflicts_of_split_face(conflict *conflict_list,
                                                                               const half_edge* face_separator,
                                                                               const vertex &extra_direction_for_split) {
            // given that a face f was just split into two faces with
            // face_separator edge, let's redistribute the conflicts
            auto * f1 = face_separator->face;
            auto * f2 = face_separator->twin->face;
            f1->conflict_list = nullptr;
            f2->conflict_list = nullptr;
            conflict * list_ref = conflict_list;
            while(list_ref!= nullptr) {
                conflict * current_ref = list_ref;  // record head
                list_ref=list_ref->next;        // advance
                current_ref->next = nullptr;    // de-attach
                auto * poly = current_ref->unadded_input_poly;
                // find the face to which the edge is classified
                auto * f = classify_conflict_against_two_faces(face_separator, poly->points[0],
                        poly->points[1], extra_direction_for_split);
                // insert edge into head of conflict list
                current_ref->next = f->conflict_list;
                f->conflict_list = current_ref;
                // pointer from edge to conflicting face
                poly->conflict_face=f;
            }
        }

        template<typename number>
        auto planarize_division<number>::classify_conflict_against_two_faces(const half_edge* face_separator,
                                                                             const vertex &c, const vertex &d,
                                                                             const vertex &extra_direction_for_split)
                                                                             -> half_edge_face *{
            // note:: edge's face always points to the face that lies to it's left.
            // 1. if the first point lie completely to the left of the edge, then they belong to f1, other wise f2
            // 2. if they lie exactly on the edge, then we test the second end-point
            // 2.1 if the second lies completely in f1, then the first vertex is in f1, otherwise f2
            // 2.2 if the second lies on the face_separator as well, choose whoever face you want, let's say f1
            const auto & a = face_separator->origin->coords;
            const auto & b = face_separator->twin->origin->coords+extra_direction_for_split;
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
            return face_separator->face; // both lie on the separator
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
                default: {// this might be problematic, we may need to report error
                    throw_regular(string_debug("is_a_before_or_equal_b_on_same_boundary:: unrecognized boundary"));
                    return false;
                }
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
            throw_regular(string_debug("locate_half_edge_of_face_rooted_at_vertex::not found face !!!"));
            return nullptr;
        }

        template<typename number>
        auto planarize_division<number>::locate_next_trapeze_boundary_vertex_from(half_edge *a,
                                                                           const trapeze_t &trapeze) -> half_edge * {
            half_edge *iter = a;
            const half_edge *end = iter;
            do {
                iter=iter->next;
                bool is_boundary=iter==trapeze.left_top || iter==trapeze.left_bottom ||
                                 iter==trapeze.right_bottom || iter==trapeze.right_top;
                if(is_boundary) return iter;
            } while(iter!=end);
            return nullptr;
        }

        template<typename number>
        auto planarize_division<number>::locate_prev_trapeze_boundary_vertex_from(half_edge *a,
                                                                           const trapeze_t &trapeze) -> half_edge * {
            half_edge *iter = a;
            const half_edge *end = iter;
            do {
                iter=iter->prev;
                bool is_boundary=iter==trapeze.left_top || iter==trapeze.left_bottom ||
                                 iter==trapeze.right_bottom || iter==trapeze.right_top;
                if(is_boundary) return iter;
            } while(iter!=end);
            return iter;
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
            // also to mention, that due to precision errors, we perform a full cone test,
            // even for more than 180 degrees, although in theory all angles are less than
            // 180, BUT, in practice, due to precision errors this will happen
            half_edge *iter = a.edge;
            const half_edge *end = iter;
            const auto root=a.coords;
            do { // we walk in CW order around the vertex
                // it seems the safest is to test against the trapeze and not just against the
                // immidiate edge
                const auto trapeze_adj= infer_trapeze(iter->face);
                const auto trapeze_prev= infer_trapeze(iter->prev->twin->face);
                if(!trapeze_adj.isDeg()) {
//                const auto prev= locate_prev_trapeze_boundary_vertex_from(iter, trapeze_adj)->origin->coords;
                    auto prev= locate_next_trapeze_boundary_vertex_from(iter->prev->twin, trapeze_prev)->origin->coords;
                    auto next= locate_next_trapeze_boundary_vertex_from(iter, trapeze_adj)->origin->coords;
                    // todo:: add safety tests if prev==root || next==root ?
                    bool inside_cone;
//                    bool is_less_than_180=classify_point(next, root, prev)<0;
//                    int cls1= classify_point(b, root, prev);
//                    int cls2= classify_point(b, root, next);
//                    if(is_less_than_180) inside_cone=cls1<=0 && cls2>0;
//                    else inside_cone=!(cls1>0 && cls2<=0);
                    //
//                    bool is_next_after_prev=classify_point(next, root, prev)<0
//                                            || (classify_point(next, root, prev)==0 && prev.dot(next)<=0);
                    auto gg=prev-root, gg2=next-root;
                    auto ll= gg.dot(gg2);
                    bool is_prev_after_next=classify_point(prev, root, next)<0
                                            || (classify_point(prev, root, next)==0
                                            && robust_dot(prev-root, next-root)>=0);//(prev-root).dot(next-root)>=0);
                    if(is_prev_after_next)
                        prev=locate_prev_trapeze_boundary_vertex_from(iter, trapeze_adj)->origin->coords;
                    //
                    bool is_next_right_of_prev=classify_point(next, root, prev)<0;
                    bool is_next_on_prev=classify_point(next, root, prev)==0;
                    bool a2= is_next_on_prev  && robust_dot(prev-root, next-root)>=0;
                    if(!a2) {
                        bool is_less_than_180=is_next_right_of_prev||a2;
                        //
                        int cls1= classify_point(b, root, prev);
                        int cls2= classify_point(b, root, next);
                        if(is_less_than_180) inside_cone=cls1<=0 && cls2>0;
                        else inside_cone=!(cls1>0 && cls2<=0);
                        // is (a,b) inside the cone so right of iter and left of next
                        if(inside_cone)
                            return iter;

                    }
                }
                iter=iter->twin->next;
            } while(iter!=end);
            throw_regular(string_debug("locate_face_of_a_b():: could not locate !!"));
            return nullptr;
        }

        template<typename number>
        auto planarize_division<number>::insert_edge_between_non_co_linear_vertices(half_edge *vertex_a_edge,
                                                                                    half_edge *vertex_b_edge,
                                                                                    const vertex &extra_direction_for_split,
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
            re_distribute_conflicts_of_split_face(e->face->conflict_list, e, extra_direction_for_split);
            // all edges of face_2 point to the old face_1, we need to change that
            walk_and_update_edges_face(e_twin, face_2);
            return e;
        }

        template<typename number>
        void planarize_division<number>::handle_co_linear_edge_with_trapeze(const trapeze_t &trapeze,
                                                                            half_edge * edge_vertex_a,
                                                                            half_edge * edge_vertex_b,
                                                                            const point_class_with_trapeze &wall_class,
                                                                            int winding,
                                                                            dynamic_pool& pool) {
            // given that (a) and (b) are on the same wall, update windings along their path/between them.
            if(winding==0) return;
            auto *start = edge_vertex_a;
            auto *end = edge_vertex_b;
            // sort if needed
            bool before = is_a_before_or_equal_b_on_same_boundary(edge_vertex_a->origin->coords,
                    edge_vertex_b->origin->coords, wall_class);
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
            // given that vertex (a) is in the closure of the trapeze
            const bool on_boundary = a_classs != point_class_with_trapeze::strictly_inside;
            const bool on_boundary_vertices = a_classs == point_class_with_trapeze::boundary_vertex;
            const bool in_left_wall = a_classs == point_class_with_trapeze::left_wall || a.x==trapeze.left_top->origin->coords.x;
            const bool in_right_wall = a_classs == point_class_with_trapeze::right_wall || a.x==trapeze.right_bottom->origin->coords.x;
            const bool in_top_wall = a_classs == point_class_with_trapeze::top_wall;
            const bool should_try_split_horizontal_trapeze_parts = !in_left_wall
                    && !in_right_wall && !on_boundary_vertices;
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
                    top_vertex_edge=
                            try_insert_vertex_on_trapeze_boundary_at({a.x,y}, trapeze,
                                    point_class_with_trapeze::top_wall,dynamic_pool);
                }
                if(bottom_vertex_edge== nullptr) {
                    auto y = evaluate_line_at_x(a.x, trapeze.left_bottom->origin->coords,
                                                trapeze.right_bottom->origin->coords);
                    bottom_vertex_edge=
                            try_insert_vertex_on_trapeze_boundary_at({a.x,y}, trapeze,
                                    point_class_with_trapeze::bottom_wall,dynamic_pool);
                }
                // in case of precision issues, bottom might be above top, if top was already present and it
                // flowed a bit during it's lifetime :)
                bottom_vertex_edge->origin->coords.y=max__(bottom_vertex_edge->origin->coords.y,
                        top_vertex_edge->origin->coords.y);

                // now, we need to split face in two
                // edge cannot exist yet because we are strictly inside horizontal part.
                // we insert a vertical edge, that starts at bottom edge into the top wall (bottom-to-top)
                auto *start_vertical_wall =
                        insert_edge_between_non_co_linear_vertices(bottom_vertex_edge,
                                top_vertex_edge, vertex{0,-1}, dynamic_pool);
                // clamp vertex to this new edge endpoints if it is before or after
                // this fights the geometric numeric precision errors, that can happen in y coords
//                clamp_vertex_horizontally(a, start_vertical_wall->origin->coords,
//                        start_vertical_wall->twin->origin->coords);
//                clamp_vertex_vertically(a, start_vertical_wall->origin->coords,
//                        start_vertical_wall->twin->origin->coords);
                // update resulting trapezes
                result.left_trapeze.right_bottom = start_vertical_wall;
                result.left_trapeze.right_top = start_vertical_wall->next;
                result.right_trapeze.left_top = start_vertical_wall->twin;
                result.right_trapeze.left_bottom = start_vertical_wall->twin->next;
                // if the vertex is on the edge boundary, it will not split of course
                bool is_zero_edge=start_vertical_wall->origin->coords==start_vertical_wall->twin->origin->coords;
                if(is_zero_edge) {
                    contract_edge(start_vertical_wall);
                    // this is delicate, since we know the edge has gone from bottom to top.
                    outgoing_vertex_edge=result.left_trapeze.right_bottom = result.left_trapeze.right_top;
                    result.right_trapeze.left_top = result.right_trapeze.left_bottom;
                } else {
                    a.y=clamp(a.y, start_vertical_wall->origin->coords.y,
                                            start_vertical_wall->twin->origin->coords.y);
                    outgoing_vertex_edge = try_split_edge_at(a, start_vertical_wall, dynamic_pool);
                }
            } // else we are on left or right walls already
            else { // we are on left or right boundary
                outgoing_vertex_edge = try_insert_vertex_on_trapeze_boundary_at(a,
                        trapeze, a_classs, dynamic_pool);
            }

            result.face_was_split = should_try_split_horizontal_trapeze_parts;
            result.vertex_a_edge_split_edge = outgoing_vertex_edge;
            result.vertex=outgoing_vertex_edge->origin;
            return result;
        }

        template<typename number>
        auto planarize_division<number>::contract_edge(half_edge * e) -> half_edge_vertex * {
            // given edge (e)=(a,b) contract it,  which is like removal and connecting it's neighbors
            auto * origin_a=e->origin;
            auto * origin_b=e->twin->origin;
            { // rewire all of (a) outgoing edges to have (b) as their origin vertex
                auto *iter = origin_a->edge;
                const auto * const end = origin_a->edge;
                do {
                    iter->origin=origin_b;
                    iter=iter->twin->next;
                } while(iter!=end);
            }
            // contract
            e->prev->next=e->next; e->next->prev=e->prev;
            e->twin->prev->next=e->twin->next; e->twin->next->prev=e->twin->prev;
            e->face->edge=e->next;
            e->twin->face->edge=e->twin->next;
            // clear
            origin_a->edge= nullptr;
            e->face= nullptr;
            origin_b->head_id=max__(origin_a->head_id, origin_b->head_id);
            return origin_b;
        }

        int id_a=-1;

        template<typename number>
        auto planarize_division<number>::handle_face_split(const trapeze_t & trapeze,
                                                           vertex &a, vertex &b, vertex extra_direction,
                                                           const point_class_with_trapeze &a_class,
                                                           const point_class_with_trapeze &b_class,
                                                           int winding,
                                                           dynamic_pool & dynamic_pool) -> face_split_result {
            // given that edge (a,b) is in the closure of the trapeze,
            // split the face to up to 4 pieces, strategy is:
            // 1. use a vertical that goes through "a" to try and split the trapeze
            // 2. if a cut has happened, two trapezes were created and "a" now lies on
            //    the vertical wall between them.
            // 3. infer the correct trapeze where "b" is in and perform roughly the same
            //    operation
            // 4. connect the two points, so they again split a face (in case they are not vertical)
            // 5. if (a,b) lies on a wall, then simply update winding between
            face_split_result result;
            result.has_horizontal_split=false;

            {
                const auto mutual_wall = do_a_b_lies_on_same_trapeze_wall(trapeze, a, b,
                                                                          a_class, b_class);
                if(mutual_wall!=point_class_with_trapeze::unknown) {
                    // co-linear, so let's just insert vertices on the boundary and handle windings
                    // we do not need to split face in this case
                    // Explain:: why don't we send vertical splits lines for boundary ? I think
                    // because the input is always a closed polyline so it doesn't matter
                    auto * edge_vertex_a =
                            try_insert_vertex_on_trapeze_boundary_at(a, trapeze, mutual_wall, dynamic_pool);
                    auto * edge_vertex_b =
                            try_insert_vertex_on_trapeze_boundary_at(b, trapeze, mutual_wall, dynamic_pool);
                    if(id_a!=-1 && edge_vertex_a->origin->id!=id_a) {
                        int n=0;
                    }
                    handle_co_linear_edge_with_trapeze(trapeze, edge_vertex_a, edge_vertex_b,
                                                       mutual_wall, winding, dynamic_pool);
                    result.planar_vertex_a=edge_vertex_a->origin;
                    result.planar_vertex_b=edge_vertex_b->origin;
                    result.has_horizontal_split=false;
                    return result;
                }
            }

            // first, try to split vertically with endpoint (a)
            vertical_face_cut_result a_cut_result = handle_vertical_face_cut(trapeze,
                    a, a_class, dynamic_pool);
            // let's find out where (b) ended up after a split (that might have not happened)
            const trapeze_t * trapeze_of_b = &trapeze;
            point_class_with_trapeze b_new_class = b_class;
            if(a_cut_result.face_was_split) {
                // a vertical split happened, let's see where b is
                bool left = b.x<=a_cut_result.vertex->coords.x;
                trapeze_of_b = left ? &a_cut_result.left_trapeze : &a_cut_result.right_trapeze;
                // now, we need to update vertex (b) class after the vertical (a) cut in a robust manner.
                //   this happens if a_cut_result.x==b.x
                if(a_cut_result.vertex->coords.x==b.x) {
                    b_new_class=left?point_class_with_trapeze::right_wall : point_class_with_trapeze::left_wall;
                    vertex wall_vertex_top=left?trapeze_of_b->right_top->origin->coords :
                            trapeze_of_b->left_top->origin->coords;
                    vertex wall_vertex_bottom=left?trapeze_of_b->right_bottom->origin->coords :
                            trapeze_of_b->left_bottom->origin->coords;
                    b.y=clamp(b.y, wall_vertex_bottom.y, wall_vertex_top.y);
                    if(wall_vertex_top==b || wall_vertex_bottom==b)
                        b_new_class=point_class_with_trapeze::boundary_vertex;
                }
            }

            // next, try to split trapeze_of_b vertically with b endpoint
            vertical_face_cut_result b_cut_result =
                    handle_vertical_face_cut(*trapeze_of_b, b, b_new_class, dynamic_pool);
            const trapeze_t *mutual_trapeze = trapeze_of_b;
            // now, infer the mutual trapeze of a and b after the vertical splits have occurred
            if(b_cut_result.face_was_split) {
                bool left = a_cut_result.vertex->coords.x<=b_cut_result.vertex->coords.x;
                mutual_trapeze = left ? &b_cut_result.left_trapeze : &b_cut_result.right_trapeze;
            }

            result.planar_vertex_a=a_cut_result.vertex; result.planar_vertex_b=b_cut_result.vertex;
            if(result.planar_vertex_a == result.planar_vertex_b)
                return result;

            // any edge of the trapeze of b will tell us the face
            const auto * face = mutual_trapeze->right_bottom->face;
            const auto loc_a=
                    locate_and_classify_vertex_that_is_already_on_trapeze(result.planar_vertex_a, *mutual_trapeze);
            const auto loc_b=
                    locate_and_classify_vertex_that_is_already_on_trapeze(result.planar_vertex_b, *mutual_trapeze);
            auto * a_edge = locate_half_edge_of_face_rooted_at_vertex(result.planar_vertex_a, face);
            auto * b_edge = locate_half_edge_of_face_rooted_at_vertex(result.planar_vertex_b, face);
            point_class_with_trapeze mutual_wall=
                    do_a_b_lies_on_same_trapeze_wall(*mutual_trapeze, result.planar_vertex_a->coords,
                            result.planar_vertex_b->coords, loc_a.classs, loc_b.classs);
            if(mutual_wall!=point_class_with_trapeze::unknown) // they are on the same wall
                handle_co_linear_edge_with_trapeze(*mutual_trapeze, a_edge, b_edge, mutual_wall, winding, dynamic_pool);
            else {
                // different walls require a split
                vertex extra_dir{0,0};
                const bool prepare_degenrate=result.planar_vertex_a->coords==result.planar_vertex_b->coords;
                if(prepare_degenrate)
                    extra_dir=extra_direction;
                auto * inserted_edge= insert_edge_between_non_co_linear_vertices(a_edge, b_edge, extra_dir, dynamic_pool);
                inserted_edge->winding = inserted_edge->twin->winding= winding;
                if(prepare_degenrate) { // we need to contract the zero edge
                    result.planar_vertex_a=result.planar_vertex_b=contract_edge(inserted_edge);
                } else {
                    result.has_horizontal_split=loc_a.classs==point_class_with_trapeze::left_wall ||
                            loc_a.classs==point_class_with_trapeze::right_wall;
                }
            }
            return result;
        }

        template<typename number>
        auto planarize_division<number>::handle_face_merge(const half_edge_vertex *vertex_on_vertical_wall) -> void {
            // given that vertex v is on a past vertical wall (left/right) boundary, we want to check 2 things:
            // 1. the vertical top/bottom edge that adjacent to it, is single(not divided) in it's adjacent trapezes
            // 2. has winding==0
            // 3. horizontal (top/bottom) wall that touches it from both side is almost a line
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
                if(index==2) break;
                iter=iter->twin->next;
            } while(iter!=end);

            // there have to be top and bottom because v was strictly inside the wall
            if(top_edge== nullptr || bottom_edge== nullptr) {
                throw_regular(string_debug("handle_face_merge::have not found bottom or top edge !!!"));
                return;
            }

            // iterate over up to two candidates
            for (int ix = 0; ix < index; ++ix) {
                auto * candidate_edge = candidates[ix];
                if(!candidate_edge) break;

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
                    // we do it by calculating the distance from vertex c to line (a,b)
                    // the illustration above is for top_edge= v-->b
//                    const auto & a = candidate_edge->next->twin->origin->coords;
//                    const auto & b = candidate_edge->twin->origin->coords;
//                    const auto & c = candidate_edge->twin->prev->origin->coords;
                    ////
                    const auto trap_left=infer_trapeze(candidate_edge->face);
                    const auto trap_right=infer_trapeze(candidate_edge->twin->face);
                    vertex a,b=candidate_edge->twin->origin->coords,c;
                    if(candidate_edge==top_edge) {
                        a=trap_left.left_top->origin->coords; c= trap_right.right_top->origin->coords;
                        if(trap_left.right_top!=candidate_edge->next) continue;
                    } else {
                        a=trap_right.right_bottom->origin->coords; c= trap_left.left_bottom->origin->coords;
                        if(trap_right.left_bottom!=candidate_edge->next) continue;
                    }
                    if(top_edge==bottom_edge) {
                        int a=0;
                    }
                    ////
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
            number numerator= abs_((b.x - a.x) * (v.y - a.y) - (v.x - a.x) * (b.y - a.y)); // 2*A
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
            walk_and_update_edges_face(edge->twin, face_1);
            // re-connect start segment
            // update the vertices edge links
            edge->origin->edge= edge->twin->next;
            edge->twin->origin->edge= edge->next;

            edge->prev->next = edge->twin->next;
            edge->twin->next->prev = edge->prev;
            // re-connect end segment
            edge->twin->prev->next = edge->next;
            edge->next->prev= edge->twin->prev;
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
                conflict_ref->unadded_input_poly->conflict_face=face_1;
                conflict_ref=conflict_ref->next;
            }
            face_2->edge=nullptr; face_2->conflict_list= nullptr;
        }

        template<typename number>
        void planarize_division<number>::insert_poly(poly_info &poly, dynamic_pool & dynamic_pool) {
            const unsigned size= poly.size;
            half_edge_vertex *last_edge_last_planar_vertex=nullptr, *first_edge_first_planar_vertex=nullptr;
            bool first_edge=true;
            for (unsigned edge = 0; edge < size; ++edge) {
                bool are_we_done = false;
                int count = 0;
                vertex a=poly.points[(edge)%size],b=poly.points[(edge+1)%size], b_tag;
                half_edge_face * face = nullptr;
                trapeze_t trapeze;
                point_class_with_trapeze class_a;
                bool last_edge= edge==size-1;
                if(last_edge) // last edge connects to first vertex in the planar polygon
                    b=first_edge_first_planar_vertex->coords;
                bool zero_edge= a==b;
                if(zero_edge) continue;
                int winding=infer_edge_winding(a, b);
                if(first_edge) {
//                    first_edge=false;
                    // first edge always needs to consult with the conflicts info
                    face=poly.conflict_face;
                    trapeze=infer_trapeze(face);
                    a=poly.points[edge];
                    class_a= round_vertex_to_trapeze(a, trapeze);
                    if(class_a!=point_class_with_trapeze::strictly_inside) {
                        // due to rounding errors, we may need to re-consider the
                        // starting trapeze.
                        const auto * e= try_insert_vertex_on_trapeze_boundary_at(a, trapeze, class_a, dynamic_pool);
                        half_edge * located_face_edge= locate_face_of_a_b(*(e->origin), b);
                        face = located_face_edge->face;
                        trapeze=infer_trapeze(face);
                        class_a = locate_and_classify_vertex_that_is_already_on_trapeze(e->origin, trapeze).classs;
                    }
                } else {
                    a=last_edge_last_planar_vertex->coords;
                    half_edge * located_face_edge= locate_face_of_a_b(*last_edge_last_planar_vertex, b);
                    face = located_face_edge->face;
                    trapeze=infer_trapeze(face);
                    class_a = locate_and_classify_vertex_that_is_already_on_trapeze(last_edge_last_planar_vertex, trapeze).classs;
                }
                const vertex direction=b-a;
                half_edge_vertex *a_planar=nullptr, *b_tag_planar=nullptr;
                while(!are_we_done) {
                    // debug
                    if(poly.id>=1 && edge==0) {
//                        if(count>=1)
//                            return;
                    }

                    // start with the conflicting face and iterate carefully, a---b'---b
                    // the reporting of and class of the next interesting point of the edge against current trapeze
                    conflicting_edge_intersection_status edge_status =
                            compute_conflicting_edge_intersection_against_trapeze(trapeze, a, b, class_a);
                    b_tag = edge_status.point_of_interest;
                    point_class_with_trapeze class_b_tag = edge_status.class_of_interest_point;
                    if(class_b_tag==point_class_with_trapeze::outside) {
                        throw_debug(string_debug("insert_edge():: class_b_tag==point_class_with_trapeze::outside !!"),
                                poly.id, edge, count);
                        return;
                    }
                    id_a=b_tag_planar ? b_tag_planar->id : -1;
                    const auto face_split_result= handle_face_split(trapeze, a, b_tag, direction, class_a,
                            class_b_tag, winding, dynamic_pool);

                    // a and b' might got rounded on the edges, so let's update
                    a_planar=face_split_result.planar_vertex_a;
                    b_tag_planar=face_split_result.planar_vertex_b;
                    { // record and stalk the first planar vertex of the polygon
                        if(first_edge){
                            first_edge=false;
                            first_edge_first_planar_vertex=a_planar;
                            first_edge_first_planar_vertex->head_id=poly.id;
                        }
                        if(a_planar->head_id==poly.id && a_planar->edge)
                            first_edge_first_planar_vertex=a_planar;
                        else if(b_tag_planar->head_id==poly.id && b_tag_planar->edge)
                            first_edge_first_planar_vertex=b_tag_planar;
                    }

                    // now, we need to merge faces if we split a vertical wall, i.e, if
                    // the new 'a' coord strictly lies in the left/right wall
                    bool candidate_merge = (count>=1) && face_split_result.has_horizontal_split;
                    if(candidate_merge&&(APPLY_MERGE))
                        handle_face_merge(a_planar);

                    if(count>=MAX_ITERATIONS)
                        throw_debug(string_debug("insert_edge():: seems like infinite looping !!"),
                                poly.id, edge, count)
                    // if b'==b we are done, this might be problematic if b' was rounded/clamped
                    are_we_done = !last_edge && (b==b_tag_planar->coords || count>=MAX_ITERATIONS);
                    if(are_we_done) {
                        last_edge_last_planar_vertex=b_tag_planar;
                        break;
                    }
                    if(last_edge && b==b_tag_planar->coords) {
                        if(b_tag_planar->head_id!=poly.id)
                            throw_debug(string_debug("last vertex didn't land on first vertex of polygon!!"),
                                    poly.id, edge, count)
                        else
                            break;

                    }
                    // ITERATION STEP, therefore update:
                    // 1. now iterate on the sub-line (b', b), by assigning a=b'
                    // 2. locate the face, that (b', b) is intruding into / conflicting
                    // 3. infer the trapeze of the face
                    // 4. infer the class of the updated vertex a=b' in this face
                    a =b_tag_planar->coords;
                    half_edge * located_face_edge= locate_face_of_a_b(*b_tag_planar, b);
                    face = located_face_edge->face;
                    trapeze=infer_trapeze(face);
                    class_a = locate_and_classify_vertex_that_is_already_on_trapeze(b_tag_planar, trapeze).classs;
                    count++;
                }

            }

        }

        template<typename number>
        void planarize_division<number>::compute(const chunker<vertex> &pieces,
                                                 const fill_rule &rule,
                                                 const tess_quality &quality,
                                                 dynamic_array<vertex> &output_vertices,
                                                 triangles::indices & output_indices_type,
                                                 dynamic_array<index> &output_indices,
                                                 dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                                                 dynamic_array<vertex> *debug_trapezes) {

            // vertices size is also edges size since these are polygons
            const auto poly_count = pieces.size();
            // plus 4s for the sake of frame
            dynamic_pool dynamic_pool{};
            // create the main frame
            auto *main_face = create_frame(pieces, dynamic_pool);
            // create edges and conflict lists
            poly_info *poly_list;
            conflict *conflict_list;
            build_poly_and_conflicts(pieces, *main_face, &poly_list, &conflict_list);

            // now start iterations
            for (index ix = 0; ix < poly_count; ++ix) {
//            for (index ix = 0; ix < 10; ++ix) {
                auto &poly = poly_list[ix];
                if(poly.size<=1) continue;
                insert_poly(poly, dynamic_pool);
            }

            delete [] poly_list;
            delete [] conflict_list;

            tessellate(dynamic_pool.getFaces().data(),
                       dynamic_pool.getFaces().size(),
                       rule, quality, output_vertices, output_indices_type,
                       output_indices, boundary_buffer, debug_trapezes);
        }

        template<typename number>
        void planarize_division<number>::tessellate(half_edge_face **faces,
                                                       index size,
                                                       const fill_rule &rule,
                                                       tess_quality quality,
                                                       dynamic_array<vertex> &output_vertices,
                                                       triangles::indices & output_indices_type,
                                                       dynamic_array<index> &output_indices,
                                                       dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                                                       dynamic_array<vertex> *debug_trapezes) {
            // compute windings of faces
            for (index ix = 0; ix < size; ++ix)
                compute_face_windings(faces[ix]);

            const bool requested_boundary_info=false;//boundary_buffer!=nullptr;
            index visible_trapezes_count=0;
            output_indices_type = boundary_buffer ? triangles::indices::TRIANGLES_WITH_BOUNDARY :
                                  triangles::indices::TRIANGLES;
            for (index ix = 0; ix < size; ++ix) {
                const half_edge_face * face = faces[ix];
                if(!infer_fill(face->winding, rule))
                    continue;
//                if(ix!=14) continue;
                visible_trapezes_count++;
                trapeze_t trapeze= infer_trapeze(face);
                if(trapeze.isDeg()) continue;
                { // add all of the vertices and record their index
                    auto *iter=face->edge;
                    do { // insert new vertices only
                        if(iter->origin->tess_index==-1)
                            iter->origin->tess_index=output_vertices.push_back(iter->origin->coords);
                        iter->origin->internal_tess_clipped=false;
                        iter=iter->next;
                    } while (iter!=face->edge);
                }
//                quality=tess_quality::prettier_with_extra_vertices;
                switch (quality) {

                    case tess_quality::fine:
                    {
                        const auto *start= trapeze.left_top;
                        auto *iter= start->next;
                        do {
                            auto *second = iter;
                            auto *third= second->next;
                            output_indices.push_back(start->origin->tess_index);
                            output_indices.push_back(second->origin->tess_index);
                            output_indices.push_back(third->origin->tess_index);
                            if(requested_boundary_info) {
                                triangles::boundary_info aa_info =
                                        triangles::create_boundary_info(
                                                second->prev==start ? !infer_fill(start->twin->face->winding, rule) : false,
                                                !infer_fill(second->twin->face->winding, rule),
                                                third->next==start ? !infer_fill(third->twin->face->winding, rule) : false
                                                );
                                boundary_buffer->push_back(aa_info);
                            }
                            iter=iter->next;
                        } while(iter->next!=start);

                    }
                        break;
                    case tess_quality::better:
                    {
                        auto *root= trapeze.left_top;
                        auto *a_edge= root->next;
                        auto *b_edge= root->prev;
#define loc(v) locate_and_classify_vertex_that_is_already_on_trapeze((v), trapeze).classs
                        point_class_with_trapeze cls_root=loc(root->origin),
                                            cls_a=loc(a_edge->origin), cls_b=loc(b_edge->origin);
                        int count=0;
                        while(a_edge!=b_edge && count++<100) {
                            // check if RAB triangle is empty on the boundary
                            bool a_b=do_a_b_lies_on_same_trapeze_wall(trapeze, a_edge->origin->coords,
                                    b_edge->origin->coords, cls_a, cls_b)!=point_class_with_trapeze::unknown;
                            bool isEmpty=(a_b && a_edge->next->origin->coords==b_edge->origin->coords)
                                    || (!a_b);
                            if(isEmpty) { // clip an ear
                                root->origin->internal_tess_clipped=true;
                                output_indices.push_back(root->origin->tess_index);
                                output_indices.push_back(a_edge->origin->tess_index);
                                output_indices.push_back(b_edge->origin->tess_index);
                                if(requested_boundary_info) {
                                    bool root_a= root->next==a_edge &&
                                                 do_a_b_lies_on_same_trapeze_wall(trapeze, root->origin->coords,
                                                         a_edge->origin->coords, cls_root, cls_a)!=point_class_with_trapeze::unknown;
                                    bool b_root=b_edge->next==root &&
                                                do_a_b_lies_on_same_trapeze_wall(trapeze, b_edge->origin->coords,
                                                        root->origin->coords, cls_b, cls_root)!=point_class_with_trapeze::unknown;
                                    root_a=root_a ? !infer_fill(root->twin->face->winding, rule) : root_a;
                                    a_b=(a_b) ? !infer_fill(a_edge->twin->face->winding, rule) : a_b;
                                    b_root=b_root ? !infer_fill(b_edge->twin->face->winding, rule) : b_root;
                                    boundary_buffer->push_back(triangles::create_boundary_info(root_a, a_b, b_root));
                                }
                            }
                            // b->r (if not clipped), r->a, a->(next available vertex)
                            if(!isEmpty) { b_edge=root; cls_b=cls_root; }
                            root=a_edge;cls_root=cls_a;
                            do {
                                a_edge=a_edge->next;
                            } while(a_edge!=b_edge && a_edge->origin->internal_tess_clipped);
                            if(a_edge->origin->internal_tess_clipped) break;
                            cls_a=loc(a_edge->origin);
                        }
#undef loc
                    }
                    break;
                    case tess_quality::prettier_with_extra_vertices:
                    {
                        const auto center = (trapeze.left_top->origin->coords + trapeze.right_top->origin->coords +
                                             trapeze.left_bottom->origin->coords + trapeze.right_bottom->origin->coords)/4;
                        int center_index= output_vertices.push_back(center);
                        const auto *start = face->edge;
                        auto *iter = start;
                        do { // triangulate the convex piece, we insert a vertex because this will create better/accurate triangles
                            auto & second= iter->origin;
                            auto & third= iter->next->origin;
                            output_indices.push_back(center_index);
                            output_indices.push_back(second->tess_index);
                            output_indices.push_back(third->tess_index);
                            if(requested_boundary_info) {
                                triangles::boundary_info aa_info =
                                        triangles::create_boundary_info(false,
                                                !infer_fill(iter->twin->face->winding, rule), false);
                                boundary_buffer->push_back(aa_info);
                            }
                            iter=iter->next;
                        } while(iter!=start);

                    }
                        break;
                }

            }

            if(debug_trapezes) { // collect trapezes so far for debug
                int count_active_faces= 0;
                for (index ix = 0; ix < size; ++ix) {
                    if(faces[ix]->isValid())
                        count_active_faces++;
                    face_to_trapeze_vertices(faces[ix], *debug_trapezes);
                }
                //std::cout<< "# active faces: " << count_active_faces <<std::endl;
            }
        }

        template<typename number>
        bool planarize_division<number>::infer_fill(int winding, const fill_rule & rule) {
            switch (rule) {
                case fill_rule::non_zero:
                    return abs_(winding);
                    break;
                case fill_rule::even_odd:
                    return abs_(winding)%2==1;
                    break;
            }
        }

        template<typename number>
        int planarize_division<number>::compute_face_windings(half_edge_face * face) {
            // given that we have an unordered graph of planar subdivision and that we have
            // already have computed windings for edges, it is enough to pick any point in the face
            // to compute the winding via a ray that spans infinitely until we are outside.
            // in a trapeze is equivalent. we will pick always the right-bottom point of the trapeze
            // and use it to infer the next penetrated trapeze and then pick again the right-bottom point.
            // this way, the path always walks right or up or down
            // while this is not exactly a linear ray, it should work because we have a planar subdivision
            if(face==nullptr || !face->isValid()) return 0;
            if(face->computed_winding) return face->winding; // memoized
            trapeze_t trapeze= infer_trapeze(face);
            const auto * point_edge = trapeze.right_bottom;
            // note this might blow up the stack
            face->winding = point_edge->winding + compute_face_windings(point_edge->twin->face);
            face->computed_winding=true; // might be redundant, but I am not taking a chance yet
            return face->winding;
        }

        template<typename number>
        number planarize_division<number>::clamp(const number &val, number a, number b) {
            if(a>b) { auto c=a;a=b;b=c; }
            if(val<a) return a;
            if(val>b) return b;
            return val;
        }

        template<typename number>
        int planarize_division<number>::infer_edge_winding(const vertex & a, const vertex & b) {
            // infer winding of edge (a,b)
            if(b.y<a.y || (b.y==a.y && b.x<a.x)) return 1; // rising/ascending edge
            else if(b.y>a.y || (b.y==a.y && b.x>a.x)) return -1; // descending edge
            return 0;
        }

        template<typename number>
        auto planarize_division<number>::do_a_b_lies_on_same_trapeze_wall(const trapeze_t & trapeze,
                                                                          const vertex &a,
                                                                          const vertex &b,
                                                                          const point_class_with_trapeze & a_class,
                                                                          const point_class_with_trapeze & b_class) -> point_class_with_trapeze {
            location_codes a_codes, b_codes;
            a_codes.compute_codes_from_class(a, a_class, trapeze); a_codes.fill();
            b_codes.compute_codes_from_class(b, b_class, trapeze); b_codes.fill();
            for (int ix = 0; ix < 4; ++ix) {
                bool share_this_wall=a_codes.array[ix]==0 && b_codes.array[ix]==0;
                if(share_this_wall) return a_codes.classOfIndex(ix);
            }
            return point_class_with_trapeze::unknown;
        }

        template<typename number>
        void planarize_division<number>::wall_vertex_endpoints(const trapeze_t & trapeze, const point_class_with_trapeze & wall,
                                                                vertex & start, vertex& end) {
            half_edge *edge_start, *edge_end;
            wall_edge_endpoints(trapeze, wall, &edge_start, &edge_end);
            start=edge_start->origin->coords; end=edge_end->origin->coords;
        }

        template<typename number>
        void planarize_division<number>::wall_edge_endpoints(const trapeze_t & trapeze, const point_class_with_trapeze & wall,
                                                        half_edge ** start, half_edge ** end) {
            switch (wall) {
                case point_class_with_trapeze::left_wall: {*start=trapeze.left_top;*end=trapeze.left_bottom;return;}
                case point_class_with_trapeze::bottom_wall: {*start=trapeze.left_bottom;*end=trapeze.right_bottom;return;}
                case point_class_with_trapeze::right_wall: {*start=trapeze.right_bottom;*end=trapeze.right_top;return;}
                case point_class_with_trapeze::top_wall: {*start=trapeze.right_top;*end=trapeze.left_top;return;}
                default: {throw_regular(string_debug("wall_endpoints():: invalid wall"));return;}
            }
        }

        template<typename number>
        auto planarize_division<number>::compute_conflicting_edge_intersection_against_trapeze(const trapeze_t & trapeze,
                                                                                               vertex &a, vertex b,
                                                                                               const point_class_with_trapeze & a_class)
        -> conflicting_edge_intersection_status {
            // given that edge (a,b), vertex (a) is conflicting, i.e on boundary or completely inside
            // and we know that the edge passes through the trapeze or lies on the boundary,
            // find the second interesting point, intersection or overlap or completely inside
            conflicting_edge_intersection_status result{};
            bool a_is_boundary_vertex=a_class==point_class_with_trapeze::boundary_vertex;
            bool a_is_interior=a_class==point_class_with_trapeze::strictly_inside;
            bool a_is_outside=a_class==point_class_with_trapeze::outside || a_class==point_class_with_trapeze::unknown;
            if(a_is_outside)
                throw_regular(string_debug("compute_conflicting_edge_intersection_against_trapeze():: a outside"));
            location_codes codes_a, codes_b;
            codes_a.compute_location_codes(a, trapeze); codes_a.fill();
            codes_b.compute_location_codes(b, trapeze); codes_b.fill();

            if(!(a_is_boundary_vertex || a_is_interior)) // todo: avoid patching,  find another solution
                codes_a.array[codes_a.indexOfClass(a_class)]=0;
            point_class_with_trapeze cla_b=point_class_with_trapeze::unknown;
            ///// over flow correction tests due to precision errors, one wall is breaking on or past a different wall
            if(!a_is_boundary_vertex) {
                int idx_start=codes_a.indexOfClass(a_class);
                point_class_with_trapeze overflow_wall=point_class_with_trapeze::unknown;
                // offset is used to skip the edge we penetrated obviously
                for (int ix = idx_start+1; ix < idx_start+4; ++ix) {
                    int idx=ix%4;
                    if(codes_a.array[idx]<=0) {
                        overflow_wall=codes_a.classOfIndex(idx); break;
                    }
                }
                if(overflow_wall!=point_class_with_trapeze::unknown) {
                    b=a;cla_b=overflow_wall;
                    result.class_of_interest_point = cla_b;
                    result.point_of_interest = b;
                    return result;
                }
            }
            ///// we got here, no overflow, let's test for insideness of b
            {
                int idx_start=codes_a.indexOfClass(a_class);
                int offset= a_is_boundary_vertex ? 0 : 1; // test 3 or 4,
                bool in_interior_of_others=true;
                for (int ix = idx_start+offset; ix < idx_start+4; ++ix) {
                    int idx=ix%4;
                    in_interior_of_others &= (codes_b.array[idx]>=0);
                }
                if(in_interior_of_others) {
                    cla_b=point_class_with_trapeze::strictly_inside;
                    offset=0;//i disable offset for on-wall detection
                    for (int ix = idx_start+offset; ix < idx_start+4; ++ix) {
                        int idx=ix%4;
                        if(codes_b.array[idx]==0) {
                            cla_b=codes_b.classOfIndex(idx);
                            break;
                        }
                    }
                }
                if(cla_b!=point_class_with_trapeze::unknown) {
                    result.class_of_interest_point = cla_b;
                    result.point_of_interest = b;
                    return result;
                }
            }
            // if we got here, then b is outside, let's try to round (a,b) edge if it overflowed
            if(!a_is_interior){
                for (int ix = 0; ix < 4; ++ix) {
                    bool suspect = codes_a.array[ix]==0 && codes_b.array[ix]<=0;
                    if(suspect) {
                        const auto wall= codes_a.classOfIndex(ix);
                        vertex start, end, v=b;
                        wall_vertex_endpoints(trapeze, wall, start, end);
                        v.x=clamp(v.x, start.x, end.x);
                        bool is_horizontal_wall= start.x!=end.x;
                        if(is_horizontal_wall)
                            v.y= evaluate_line_at_x(v.x, start, end);
                        v.y=clamp(v.y, start.y, end.y);
                        result.class_of_interest_point=wall;
                        result.point_of_interest=clamp_vertex_to_trapeze_wall(v,
                                wall, trapeze);
                        if(result.point_of_interest==start || result.point_of_interest==end)
                            result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                        return result;
                    }
                }
            }

            if(trapeze.isDeg()) {
                int a=5;
            }
            // if we got here, vertex (a) is in the closure and vertex (b) is outside/exterior.
            // the strategy is to behave like cohen-sutherland
            vertex intersection_point{};
            number alpha(0), alpha1(0); // alphas are not reliable because of the way I compute
            intersection_status status;

            // i designed the intersection to not being robust for extreme sub-pixel values,
            // in this case, I rely on the accurate codes, when an intersection, that should have
            // been reported was not reported. The reason why I am against robust intersection is because
            // of potential overflow issues
            if(!a_is_boundary_vertex) {
                int idx_start=codes_a.indexOfClass(a_class);
                codes_a.array[idx_start]=0;
            }
            for (int ix = 0; ix < 4; ++ix) {
                bool suspect= codes_a.array[ix] * codes_b.array[ix] == -1;
                if(!suspect) continue;
                const auto wall=codes_a.classOfIndex(ix); vertex start, end;
                wall_vertex_endpoints(trapeze, wall, start, end);
                result.class_of_interest_point = wall;
                status = finite_segment_intersection_test(a, b, start, end, intersection_point, alpha, alpha1);
                bool found_intersection=status==intersection_status::intersect;
                result.point_of_interest=found_intersection ? intersection_point : b;
                result.point_of_interest=clamp_vertex_to_trapeze_wall(result.point_of_interest, wall, trapeze);
                if(result.point_of_interest==start || result.point_of_interest==end)
                    result.class_of_interest_point = point_class_with_trapeze::boundary_vertex;
                if(found_intersection) break;
            }

            //////
            if(result.class_of_interest_point==point_class_with_trapeze::unknown)
                result.class_of_interest_point= point_class_with_trapeze::strictly_inside;
            return result;
        }

        template<typename number>
        auto planarize_division<number>::finite_segment_intersection_test(const vertex &a, const vertex &b,
                                                                          const vertex &c, const vertex &d,
                                                                          vertex & intersection,
                                                                          number &alpha, number &alpha1) -> intersection_status{
            // this procedure will find proper and improper(touches) intersections, but no
            // overlaps, since overlaps induce parallel classification, this would have to be resolved outside,
            // this is NOT sub-pixel robust when underflows occur, but I don't need it that robust
            if(a==b || c==d) return intersection_status::degenerate_line;
            auto ab = b - a;
            auto cd = d - c;
            auto dem = ab.x * cd.y - ab.y * cd.x;
            // parallel lines
            if (abs_(dem) <= number(0)) return intersection_status::parallel;
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
                if(numerator_1==0) { alpha=0; intersection = a; }// a lies on c--d segment
                else if(numerator_1==dem) { alpha=1; intersection = b;}// b lies on c--d segment
                else if(numerator_2==0) { alpha=0; intersection = c;}// c lies on a--b segment
                else if(numerator_2==dem) { alpha=1; intersection = d;}// d lies on a--b segment
                else { // proper intersection
                    alpha = numerator_1/dem;
                    // alpha1 = numerator_2/dem;
                    intersection = a + (ab*numerator_1)/dem;
                }
            }
            return intersection_status::intersect;
        }
int PP=16;
        template <typename number>
        inline int
        planarize_division<number>::classify_point(const vertex & point, const vertex &a, const vertex & b) {
            auto a_p=point-a, a_b=b-a;
            auto result2= robust_dot(a_b, a_p.orthogonalLeft());
//            auto result2= (a_b*PP).dot(a_p.orthogonalLeft()*PP);
            if(result2<0) return 1;
            else if(result2>0) return -1;
            else return 0;
//
//            auto a_p=point-a, a_b=b-a;
//            auto result2= (a_b*PP).dot(a_p.orthogonalLeft()*PP);
//            if(result2<0) return 1;
//            else if(result2>0) return -1;
//            else return 0;

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


        template <typename number>
        inline number
        planarize_division<number>::robust_dot(const vertex &u, const vertex & v) {
            int f1=1, f2=1;
            number w_u=u.x<0?(-u.x):u.x;
            number h_u=u.y<0?(-u.y):u.y;
            if(w_u>0 && w_u<number(1)) f1=int(number(2)/w_u);
            if(h_u>0 && h_u<number(1)) f2=int(number(2)/h_u);
//            int f=16;
            int f=f1>f2?f1:f2;
            auto result= (u * f).dot(v * f);
            return result;
        }
// 3*2 - (3*0)
    }
}
