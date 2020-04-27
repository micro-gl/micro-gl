
//#include "stroke_tessellation.h"

namespace microgl {

    namespace tessellation {

        template<typename number>
        bool stroke_tessellation<number>::rays_intersect(
                const vertex &a,
                const vertex &b,
                const vertex &c,
                const vertex &d,
                vertex &intersection
        ) {
            // given, that infinite lines that pass through segments (a,b) and (c,d)
            // are not parallel, compute intersection.
            vertex s1 = b - a;
            vertex s2 = d - c;
            vertex dc = a - c;
            // we reduce the precision with multiplications to avoid extreme overflow
            number det = -(s2.x * s1.y) + (s1.x * s2.y);
            number s = -(s1.y * dc.x) + (s1.x * dc.y);
            number t = (s2.x * dc.y) - (s2.y * dc.x);
            if (det!=0) intersection = a + (s1*t)/det;
            return det!=0;
        }

        template<typename number>
        auto stroke_tessellation<number>::finite_segment_intersection_test(const vertex &a, const vertex &b,
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

        template<typename number>
        void stroke_tessellation<number>::comp_parallel_ray(
                const vertex &pt0,
                const vertex &pt1,
                vertex &pt_out_0,
                vertex &pt_out_1,
                number stroke) {
            vertex vec = pt1 - pt0;
            vertex normal = {vec.y, -vec.x};
            number length = microgl::math::length(normal.x, normal.y);
            vertex dir = (normal*stroke)/length;
            pt_out_0 = pt0 + dir;
            pt_out_1 = pt1 + dir;
        }

        template<typename number>
        auto stroke_tessellation<number>::build_quadrilateral(const stroke_tessellation::vertex &a,
                                                              const stroke_tessellation::vertex &b,
                                                              const number &stroke_width) -> poly_4 {
            poly_4 result;
            comp_parallel_ray(a, b, result.left, result.top, stroke_width);
            vertex dir=a-result.left;
            result.right=b+dir;
            result.bottom=a+dir;
            return result;
        }

        template <typename number>
        inline int
        stroke_tessellation<number>::classify_point(const vertex & point, const vertex &a, const vertex & b) {
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

        template<typename number>
        auto stroke_tessellation<number>::compute_distanced_tangent_at_joint(
                const vertex &a, const vertex &b, const vertex &c, number mag
        ) -> edge {
            edge result;
            vertex tangent1 = b-a, tangent2 = c-b;
            vertex normal1 = {tangent1.y, -tangent1.x}, normal2 = {tangent2.y, -tangent2.x};
            normal1 = normal1/microgl::math::length(normal1.x, normal1.y);
            normal2 = normal2/microgl::math::length(normal2.x, normal2.y);
            vertex pre_normal_join= normal1+normal2;
            if(pre_normal_join==vertex{0,0}) {
                pre_normal_join=b-a;
                mag= mag<0 ? -mag : mag;
            }
            const number length= microgl::math::length(pre_normal_join.x, pre_normal_join.y);
            const vertex direction=(pre_normal_join*mag)/length;
            result.a = b + direction;
            result.b = result.a + vertex{direction.y, -direction.x};
            return result;
        }

        template<typename number>
        auto stroke_tessellation<number>::resolve_left_right_walls_intersections(
                const poly_4 &a, const poly_4 &b) -> poly_inter_result {
            poly_inter_result result{};
            vertex result_inter;
            number alpha;
            // test left walls
            auto status= finite_segment_intersection_test(a.left, a.top, b.left, b.top,
                            result_inter, alpha, alpha);
            if(status==intersection_status::parallel) {
                if(a.top==b.left) {
                    result.left= a.top;
                    result.has_left=true;
                    result.right= a.right;
                    result.has_right=true;
                    return result;
                }
            } else if (status==intersection_status::intersect) {
                result.left= result_inter;
                result.has_left=true;
                return result;
            }
            // right walls
            status= finite_segment_intersection_test(a.right, a.bottom, b.right, b.bottom,
                                                          result_inter, alpha, alpha);
            if (status==intersection_status::intersect) {
                result.right= result_inter;
                result.has_right=true;
            }
            return result;
        }

        template<typename number>
        void stroke_tessellation<number>::compute(
                number stroke_width,
                bool closePath,
                stroke_cap cap,
                stroke_line_join line_join,
                const stroke_gravity gravity,
                const vertex *points,
                const index size,
                dynamic_array<vertex> &output_vertices,
                dynamic_array<index> &output_indices,
                microgl::triangles::indices &output_indices_type,
                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer,
                number miter_limit)
                {

            bool with_boundary =boundary_buffer!= nullptr;
            output_indices_type=triangles::indices::TRIANGLES_STRIP;
            number stroke_size = max_(number(1), stroke_width / number(2));
            poly_4 current, next;
            current = build_quadrilateral(points[0], points[1], stroke_size);
            current.left_index= output_vertices.push_back(current.left);
            current.bottom_index= output_vertices.push_back(current.bottom);
            if(!closePath)
                apply_cap(cap, false, points[0], current.bottom_index, current.left_index,
                        stroke_size, output_vertices, output_indices, boundary_buffer);
            output_indices.push_back(current.left_index);
            output_indices.push_back(current.bottom_index);
            // if the path should close we cyclically extend to the second segment.
            const index segments= closePath ? size+1 : size-1;
            for (index ix = 1; ix < segments; ++ix) {
                next = build_quadrilateral(points[ix%size], points[(ix+1)%size], stroke_size);
                auto res= resolve_left_right_walls_intersections(current, next);
                // use relevant vertices and store indices
                if(res.has_left) {
                    res.left_index= output_vertices.push_back(res.left);
                } else {
                    current.top_index =output_vertices.push_back(current.top);
                    next.left_index =output_vertices.push_back(next.left);
                }
                if(res.has_right) {
                    res.right_index= output_vertices.push_back(res.right);
                } else {
                    current.right_index =output_vertices.push_back(current.right);
                    next.bottom_index =output_vertices.push_back(next.bottom);
                }

                output_indices.push_back(res.has_left ? res.left_index : current.top_index);
                // right wall intersection or first point on current right wall
                output_indices.push_back(res.has_right ? res.right_index : current.right_index);
                // line join
                bool skip_join= res.has_right && res.has_left; // straight line
//                line_join = stroke_line_join::round;
                if(!skip_join) {
//                if(false&&!skip_join) {
                    // now walk to hinge
                    const vertex join_vertex= points[ix%size];
                    index join_index = output_vertices.push_back(join_vertex);
                    // todo: first and last index might not be correct for when I force no intersections,
                    // todo: in this case I should work it out (should be easy)
                    index first_index= res.has_right ? current.top_index : current.right_index;
                    index last_index= res.has_right ? next.left_index : next.bottom_index;
                    /////
                    bool cls= classify_point(next.left, current.top, join_vertex)>=0;
                    if(cls) {
                        first_index= current.top_index; last_index=next.left_index;
                    } else {
                        first_index= next.bottom_index; last_index=current.right_index;
                    }
                    /////

                    output_indices.push_back(join_index);
                    output_indices.push_back(join_index);
                    output_indices.push_back(first_index);
                    output_indices.push_back(join_index);
                    apply_line_join(line_join, res.has_right, current, next, first_index, join_index, last_index,
                                    stroke_size, miter_limit, output_vertices, output_indices, boundary_buffer);
                    // close the join and reinforce
                    output_indices.push_back(last_index);
                    output_indices.push_back(join_index);
                    output_indices.push_back(join_index);
                }
                // second quad
                // left wall intersection or last point on current left wall
                output_indices.push_back(res.has_left ? res.left_index : next.left_index);
                // right wall intersection or first point on current right wall
                output_indices.push_back(res.has_right ? res.right_index : next.bottom_index);
                current=next;
            }

            // close the last segment
            if(!closePath) {
                current.top_index= output_vertices.push_back(current.top);
                current.right_index= output_vertices.push_back(current.right);
                output_indices.push_back(current.top_index);
                output_indices.push_back(current.right_index);
                output_indices.push_back(current.right_index);
                apply_cap(cap, false, points[size-1], current.top_index, current.right_index,
                          stroke_size, output_vertices, output_indices, boundary_buffer);
            }

            if(closePath) {
                // close path requires some special things, like adjusting the first segment
                // some indices from the last segment
                const auto count= output_indices.size();
                output_indices[0]=output_indices[count-2];
                output_indices[1]=output_indices[count-1];
            }
        }

        template<typename number>
        void stroke_tessellation<number>::apply_line_join(
                const stroke_line_join &line_join,
                const bool has_right,
                const poly_4 & current,
                const poly_4 & next,
                const index & first_index,
                const index & join_index,
                const index & last_index,
                const number &stroke_size,
                const number &miter_limit,
                dynamic_array<vertex> &output_vertices,
                dynamic_array<index> &output_indices,
                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer) {
            // insert vertices strictly between the start and last vertex of the join
            const auto first_vertex = output_vertices[first_index];
            const auto join_vertex = output_vertices[join_index];
            const auto last_vertex = output_vertices[last_index];
            number miter_threshold = miter_limit*stroke_size;

            switch (line_join) {
                case stroke_line_join::none:
                    output_indices.push_back(join_index);
                    break;
                case stroke_line_join::miter:
                case stroke_line_join::miter_clip:
                {
                    // 1. find the clipping point
                    // 2. find the clipping line
                    // 3.
                    const edge clip_ray=
                            compute_distanced_tangent_at_joint(first_vertex, join_vertex, last_vertex,
                                                               miter_threshold);
//                                                               has_right ? miter_threshold : -miter_threshold);
                    edge a_ray=has_right ? edge{current.left, current.top} :
                               edge{current.bottom, current.right};
                    edge b_ray=has_right ? edge{next.top, next.left} :
                               edge{next.right, next.bottom};
                    /////
                    a_ray = {first_vertex, first_vertex+(join_vertex-first_vertex).ortho()};
                    b_ray = {last_vertex, last_vertex+(join_vertex-last_vertex).ortho()};
                    /////
                    vertex intersection{};
                    number alpha;
                    // clip rays against the clip ray, so miter will avoid overflows
                    rays_intersect(a_ray.a, a_ray.b, clip_ray.a, clip_ray.b, a_ray.b);
                    rays_intersect(b_ray.a, b_ray.b, clip_ray.a, clip_ray.b, b_ray.b);
                    // now compute intersection
                    auto status=
                            finite_segment_intersection_test(a_ray.a, a_ray.b, b_ray.a, b_ray.b,
                                                             intersection, alpha, alpha);
                    if(status==intersection_status::intersect) {
                        // found intersection inside the half clip space
                        output_indices.push_back(output_vertices.push_back(intersection));
                        output_indices.push_back(join_index);
                    } else if(line_join==stroke_line_join::miter_clip) {
                        // no found intersection inside the half clip space, so we use
                        // the two clipped points calculated
                        output_indices.push_back(output_vertices.push_back(a_ray.b));
                        output_indices.push_back(join_index);
                        output_indices.push_back(output_vertices.push_back(b_ray.b));
                        output_indices.push_back(join_index);
                    } else {
                        // else regular miter falls back to bevel (no vertices) if no intersection
                    }
                    break;
                }
                case stroke_line_join::round:
                {
                    compute_arc(first_vertex, join_vertex, last_vertex,
                                stroke_size, 100/4,
//                                has_right ? stroke_size : -stroke_size, 100,
                                join_index, output_vertices, output_indices, boundary_buffer);
                    break;
                }
                case stroke_line_join::bevel:
                    // bevel, add no point in between
                    break;
            }
        }

        template<typename number>
        void stroke_tessellation<number>::apply_cap(
                const stroke_cap &cap,
                const bool is_start,
                const vertex &root,
                const index & a_index,
                const index & b_index,
                const number &radius,
                dynamic_array<vertex> &output_vertices,
                dynamic_array<index> &output_indices,
                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer) {
            const auto a= output_vertices[a_index];
            const auto b= output_vertices[b_index];

            switch (cap) {
                case stroke_cap::butt:
                    // do nothing
                    break;
                case stroke_cap::round:
                {
                    const index root_index= output_vertices.push_back(root);
                    output_indices.push_back(a_index);
                    output_indices.push_back(root_index);
                    compute_arc(a, root, b,
                                radius, 100,
                                root_index, output_vertices, output_indices, boundary_buffer);
                    output_indices.push_back(b_index);
                    output_indices.push_back(root_index);
//                    output_indices.push_back(a_index); // close it
                    break;
                }
                case stroke_cap::square:
                {
                    const auto dir=b-a;
                    const auto n=vertex{dir.y, -dir.x};
                    const auto dir2 = (n*radius)/microgl::math::length(n.x, n.y);
                    output_indices.push_back(output_vertices.push_back(a+dir2));
                    output_indices.push_back(output_vertices.push_back(a+dir2));
                    output_indices.push_back(output_vertices.push_back(b+dir2));
                    output_indices.push_back(a_index);
                    output_indices.push_back(b_index);
                    break;
                }
            }
        }

        template<typename number>
        void stroke_tessellation<number>::compute_arc(
                const vertex &from, const vertex &root, const vertex &to,
                const number &radius,
                const number &max_distance_squared,
                const index & root_index,
                dynamic_array<vertex> &output_vertices,
                dynamic_array<index> &output_indices,
                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer) {
            // this procedure will generate points on a half circle adaptively
            const auto dir= to-from;
            bool done= (dir.x*dir.x + dir.y*dir.y)<=max_distance_squared;
            if(done) return;
            const auto e= compute_distanced_tangent_at_joint(from, root, to, radius);
            bool not_in_cone_precision_issues= classify_point(e.a, root, from)>=0 || classify_point(e.a, root, to)<=0;
            if(not_in_cone_precision_issues) return; // in case, the bisecting point was not in cone
            compute_arc(from, root, e.a, radius, max_distance_squared, root_index,
                        output_vertices, output_indices, boundary_buffer);
            output_indices.push_back(output_vertices.push_back(e.a));
            output_indices.push_back(root_index);
            compute_arc(e.a, root, to, radius, max_distance_squared, root_index,
                        output_vertices, output_indices, boundary_buffer);
        }

    }

}