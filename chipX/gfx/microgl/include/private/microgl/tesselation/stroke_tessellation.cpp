
#include "stroke_tessellation.h"

namespace microgl {

    namespace tessellation {

        template<typename number>
        bool stroke_tessellation<number>::non_parallel_rays_intersect(
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

            if (det) {
                // try this later
                // we mix 64 bit precision to avoid overflow
//            number dir_x = s1.x*(t/det);
//            number dir_y = s1.y*(t/det);
                number dir_x = (s1.x * t) / det;
                number dir_y = (s1.y * t) / det;

                vertex dir{dir_x, dir_y};
                intersection = a + dir;

                return true;
            }

            return false;
        }

        template<typename number>
        void stroke_tessellation<number>::merge_intersection(
                const vertex &a,
                const vertex &b,
                const vertex &c,
                const vertex &d,
                vertex &merge_out
        ) {
            bool co_linear = !non_parallel_rays_intersect(
                    a, b, c, d,
                    merge_out);
            if (co_linear)
                merge_out = b;
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
            vertex normal = {-vec.y, vec.x};
//        number length = microgl::math::sqrt((normal.x*normal.x) + (normal.y*normal.y));
            number length = microgl::math::length(normal.x, normal.y);
            number dir_x = (normal.x * stroke) / length;
            number dir_y = (normal.y * stroke) / length;
            vertex dir{dir_x, dir_y};
            pt_out_0 = pt0 - dir;
            pt_out_1 = pt1 - dir;
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
                number stroke_size,
                bool closePath,
                const stroke_gravity gravity,
                const vertex *points,
                const index size,
                dynamic_array<vertex> &output_vertices,
                dynamic_array<index> &output_indices,
                microgl::triangles::indices &output_indices_type,
                dynamic_array<microgl::triangles::boundary_info> *boundary_buffer)
                {

            bool with_boundary =boundary_buffer!= nullptr;
            index idx=output_vertices.size();
            output_indices_type=triangles::indices::TRIANGLES_STRIP;
            stroke_size = max_(number(1), stroke_size / number(2));
            poly_4 current, next;
            current = build_quadrilateral(points[0], points[1], stroke_size);
            output_vertices.push_back(current.left);
            output_vertices.push_back(current.bottom);
            output_indices.push_back(idx++);
            output_indices.push_back(idx++);
            for (index ix = 1; ix < size - 1; ++ix) {
                next = build_quadrilateral(points[ix], points[ix+1], stroke_size);
                auto res= resolve_left_right_walls_intersections(current, next);
                // left wall intersection or last point on current left wall
                output_vertices.push_back(res.has_left ? res.left : current.top);
                output_indices.push_back(idx++);
                // right wall intersection or first point on current right wall
                output_vertices.push_back(res.has_right ? res.right : current.right);
                output_indices.push_back(idx++);
                // now walk to hinge
                vertex hinge = (current.top+current.right)/2; //also equals current and next top intersections
                index hinge_index = output_vertices.push_back(hinge);
                output_indices.push_back(idx++);
                output_indices.push_back(hinge_index); // reinforce hinge and degenarate
                if(hinge_index!=idx-1) return;
                // add join HERE
                //
                output_indices.push_back(hinge_index); // reinforce hinge and degenarate
                // second quad
                // left wall intersection or last point on current left wall
                output_vertices.push_back(res.has_left ? res.left : next.left);
                output_indices.push_back(idx++);
                // right wall intersection or first point on current right wall
                output_vertices.push_back(res.has_right ? res.right : next.bottom);
                output_indices.push_back(idx++);

                current=next;
            }

            output_vertices.push_back(current.top);
            output_vertices.push_back(current.right);
            output_indices.push_back(idx++);
            output_indices.push_back(idx++);

        }


    }

}