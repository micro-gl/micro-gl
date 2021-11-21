/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "vec2.h"
#include "triangles.h"
#include "traits.h"
#include "math.h"

namespace microtess {

    enum class stroke_cap { butt, round, square };
    enum class stroke_line_join { none, miter, miter_clip, round, bevel };
    enum class stroke_gravity { center, inward, outward };

    /**
     * Stroke Tessellation
     *
     * @tparam number number type of vertices
     * @tparam container_output_vertices container type to store vertices
     * @tparam container_output_indices container type to store indices
     * @tparam container_output_boundary container type to store boundary info
     */
    template<typename number, class container_output_vertices,
            class container_output_indices,
            class container_output_boundary>
    class stroke_tessellation {
    public:
        using vertex = microtess::vec2<number>;
        using index = unsigned int;
        using boundary_info = microtess::triangles::boundary_info;
        using precision = unsigned char;

    private:
        struct edge { vertex a, b; };
        struct poly_4 {
            // clock wise around the bone
            vertex left{}, top{}, right{}, bottom{};
            int left_index=-1, top_index=-1, right_index=-1, bottom_index=-1;
        };
        struct poly_inter_result {
            vertex left{}, right{};
            int left_index=-1, right_index=-1;
            bool has_left=false, has_right=false;
        };

    public:
        stroke_tessellation()=delete;
        stroke_tessellation(const stroke_tessellation &)=delete;
        stroke_tessellation(stroke_tessellation &&)=delete;
        stroke_tessellation & operator=(const stroke_tessellation &)=delete;
        stroke_tessellation & operator=(stroke_tessellation &&)=delete;
        ~stroke_tessellation()=delete;

#define abs__(x) (((x)<0)?(-(x)):(x))
#define min__(a,b) (((a)<(b))?(a):(b))
#define max__(a, b) ((a)>(b) ? (a) : (b))

        /**
         * SFIANE to support allocator aware containers and non-aware containers such
         * as static arrays
         */
        template<bool on=false> struct construct_for_allocator_aware_t {
            const container_output_vertices & copy;
            explicit construct_for_allocator_aware_t(const container_output_vertices & vv) :
                    copy{vv} {}
            container_output_vertices create() { return container_output_vertices(); }
        };
        template<> struct construct_for_allocator_aware_t<true> {
            const container_output_vertices & copy;
            explicit construct_for_allocator_aware_t(const container_output_vertices & vv) :
                copy{vv} {}
            container_output_vertices create() {
                return container_output_vertices(copy.get_allocator());
            }
        };

        template<class iterable> static void compute_with_dashes(const number &stroke_width,
                                 bool closePath,
                                 const stroke_cap &cap,
                                 const stroke_line_join &line_join,
                                 const number &miter_limit,
                                 const iterable & stroke_dash_array,
                                 int stroke_dash_offset,
                                 const vertex *points,
                                 index size,
                                 container_output_vertices &output_vertices,
                                 container_output_indices &output_indices,
                                 triangles::indices &output_indices_type,
                                 container_output_boundary *boundary_buffer= nullptr) {
            int offset=stroke_dash_offset, sum_dashes=0;
            index dash_arr_length=stroke_dash_array.size()*2;
            int dash_array[dash_arr_length];
            { // copy to an even array
                int ix=0;
                for (auto& dash : stroke_dash_array) {
                    dash_array[ix]=dash_array[ix+stroke_dash_array.size()]=dash;
                    ix++;
                }
            }
            // sum of dashes
            for (index ix = 0; ix < dash_arr_length; ++ix)
                sum_dashes+=dash_array[ix];
            // if sum is zero, bail out quickly
            if(sum_dashes==0) {
                compute(stroke_width, closePath, cap, line_join, miter_limit,
                        points, size, output_vertices,
                        output_indices, output_indices_type, boundary_buffer);
                return;
            }

            offset=offset<0 ? (sum_dashes-(abs__(offset)%sum_dashes)) : offset%sum_dashes;
            int dash_index=0,  current_seg=0;
            // total length up to current segment
            number total_length=0, path_length=0;
            const index segments_count= closePath ? size+0 : size-1;
            // calculate path length
            for (index ix = 0; ix < segments_count; ++ix) {
                const auto vec= points[(ix+1)%size]-points[ix%size];
                path_length+=norm(vec);
                if(ix==0) total_length=path_length; // adjusted to reflect current segment
            }

            // SFIANE
            // we use this method to detect if container_output_vertices is allocator-aware container:
            // 1. if so, construct points_segments with the allocator of container_output_vertices.
            // 2. otherwise, use the default constructor of the container
            constexpr bool _is_allocator_aware =
                    microtess::traits::is_allocator_aware<container_output_vertices>::value;
            construct_for_allocator_aware_t<_is_allocator_aware> construct{output_vertices};
            container_output_vertices points_segments = construct.create();

            number dash_length, position;

            // calculate first index
            if(!closePath) {
                // calculate the prefix
                int partial_sum=0;
                for (index ix = 0; ix < dash_arr_length; ++ix) {
                    partial_sum+=dash_array[ix];
                    if(partial_sum>=offset) {
                        dash_index=ix; break;
                    }
                }
                dash_length= min__(number(partial_sum-offset), path_length);
                position=0;
            } else {
                // make it cyclic
                position=offset; dash_length=dash_array[0];
                path_length+=offset-dash_array[dash_arr_length-1];
            }

            while (position<path_length) { // build the points buffers
                points_segments.clear();
                const bool stroke=(dash_index%2)==0;
                if(stroke) {
                    number positions[2] = {position, position+dash_length};
                    for (int ix = 0; ix < 2; ++ix) {
                        while(positions[ix]>total_length) {
                            current_seg++;
                            const auto seg_vec= (points[(current_seg+1)%size]- points[(current_seg)%size]);
                            const auto seg_len=norm(seg_vec); total_length+=seg_len;
                            if(ix==1 && seg_len) points_segments.push_back(points[current_seg%size]);
                        }
                        // compute start-point
                        vertex seg_vec= (points[(current_seg+1)%size]- points[(current_seg)%size]);
                        number seg_len=norm(seg_vec);
                        const auto point = points[(current_seg)%size] + (seg_len==0 ? vertex{0,0} :
                                (seg_vec*(positions[ix]-(total_length-seg_len)))/seg_len);
                        points_segments.push_back(point);
                    }
                    compute(stroke_width, false, cap, line_join, miter_limit,
                            points_segments.data(), points_segments.size(), output_vertices,
                            output_indices, output_indices_type, boundary_buffer);
                }
                // find new position markers
                position+=dash_length;
                dash_index=(dash_index+1)%dash_arr_length;
                dash_length= min__(number(dash_array[dash_index]), path_length-position);
            }
        }

        static void compute(const number &stroke_width,
                     bool closePath,
                     const stroke_cap &cap,
                     const stroke_line_join &line_join,
                     const number &miter_limit,
                     const vertex *points,
                     index size,
                     container_output_vertices &output_vertices,
                     container_output_indices &output_indices,
                     triangles::indices &output_indices_type,
                     container_output_boundary *boundary_buffer= nullptr) {
#define b1 b1_(boundary_buffer, output_indices)
#define b2 b2_(boundary_buffer, output_indices)
#define reinforce reinforce_(boundary_buffer, output_indices)
            reinforce; // reinforce last index, for degenerate transitioning
            if(size==0) return;
            const auto indices_offset= output_indices.size();
            output_indices_type=triangles::indices::TRIANGLES_STRIP;
//            number stroke_size = max__(number(1), stroke_width / number(2));
            number stroke_size = stroke_width / number(2);
            index start_index=0; // find the first non-degenerate index
            while((start_index+1<size) && points[start_index]==points[start_index+1]) {
                start_index++;
            }
            poly_4 current, next;
            current = build_quadrilateral(points[start_index], points[(start_index+1)%size], stroke_size);
            output_vertices.push_back(current.left); current.left_index= output_vertices.size()-1;
            output_vertices.push_back(current.bottom); current.bottom_index= output_vertices.size()-1;

            if(!closePath)
                apply_cap(cap, false, points[start_index], current.bottom_index, current.left_index,
                          stroke_size, output_vertices, output_indices, boundary_buffer);
            // reinforce again
            output_indices.push_back(current.left_index); index first_left_index = output_indices.size()-1 ;b1;
            output_indices.push_back(current.left_index); b1;
            output_indices.push_back(current.bottom_index); b1;
            index b_first_edge_idx= boundary_buffer && boundary_buffer->size() ? boundary_buffer->size()-1 : 0;
            // if the path should close we cyclically extend to the second segment.
            const index segments= closePath ? size+1 : size-1;
            for (index ix = start_index+1; ix < segments; ++ix) {
                const vertex start= points[ix%size], end= points[(ix+1)%size];
                if(start==end) continue; // skip degenerate segments
                next = build_quadrilateral(start, end, stroke_size);
                auto res= resolve_left_right_walls_intersections(current, next);
                // use relevant vertices and store indices
                if(res.has_left) {
                    output_vertices.push_back(res.left); res.left_index= output_vertices.size()-1;
                } else {
                    output_vertices.push_back(current.top); current.top_index =output_vertices.size()-1;
                    output_vertices.push_back(next.left); next.left_index =output_vertices.size()-1;
                }
                if(res.has_right) {
                    output_vertices.push_back(res.right); res.right_index= output_vertices.size()-1;
                } else {
                    output_vertices.push_back(current.right); current.right_index =output_vertices.size()-1;
                    output_vertices.push_back(next.bottom); next.bottom_index =output_vertices.size()-1;
                }

                output_indices.push_back(res.has_left ? res.left_index : current.top_index); b2;
                // right wall intersection or first point on current right wall
                output_indices.push_back(res.has_right ? res.right_index : current.right_index); b2;
                // line join
                bool skip_join= res.has_right && res.has_left; // straight line
                if(!skip_join) {
                    // now walk to hinge
                    const vertex join_vertex= start;
                    output_vertices.push_back(join_vertex); index join_index = output_vertices.size()-1;
                    bool cls= classify_point(next.left, current.top, join_vertex)>=0;
                    int first_index= cls ? current.top_index : next.bottom_index;
                    int last_index= cls ? next.left_index : current.right_index;
                    if(first_index!=-1 && last_index!=-1) {
                        output_indices.push_back(join_index); b1;
                        output_indices.push_back(join_index); b1;
                        output_indices.push_back(first_index); b1;
                        output_indices.push_back(join_index); b1;
                        apply_line_join(line_join, first_index, join_index, last_index, stroke_size,
                                        miter_limit, output_vertices, output_indices, boundary_buffer);
                        // close the join and reinforce
                        output_indices.push_back(last_index);b2;
                        output_indices.push_back(join_index);b1;
                        output_indices.push_back(join_index);b1;                    }

                }
                // second quad
                // left wall intersection or last point on current left wall
                output_indices.push_back(res.has_left ? res.left_index : next.left_index); b1;
                // right wall intersection or first point on current right wall
                output_indices.push_back(res.has_right ? res.right_index : next.bottom_index); b1;
                current=next;
            }

            // close the last segment
            if(!closePath) {
                output_vertices.push_back(current.top); current.top_index= output_vertices.size()-1;
                output_vertices.push_back(current.right); current.right_index= output_vertices.size()-1;
                output_indices.push_back(current.top_index); b2;
                output_indices.push_back(current.right_index); b2; index b_idx=boundary_buffer ? boundary_buffer->size()-1 : 0;
                output_indices.push_back(current.right_index); b1;
                apply_cap(cap, false, points[size-1], current.top_index, current.right_index,
                          stroke_size, output_vertices, output_indices, boundary_buffer);
                if(cap==stroke_cap::butt && boundary_buffer) {
                    (*boundary_buffer)[b_idx] = triangles::create_boundary_info(false, true, true);
                    (*boundary_buffer)[b_first_edge_idx+1] = triangles::create_boundary_info(true, false, true);
                }
            }

            if(closePath) {
                // close path requires some special things, like adjusting the first segment
                // some indices from_sampler the last segment
                const auto count= output_indices.size();
                output_indices[first_left_index+0]=output_indices[count-2];
                output_indices[first_left_index+1]=output_indices[count-2];
                output_indices[first_left_index+2]=output_indices[count-1];
            }
#undef reinforce
#undef b1
#undef b2
#undef abs__
#undef min__
#undef max__
        }

    private:
        enum class intersection_status { intersect, none, parallel, degenerate_line };

        static number norm(const vertex & p) {
            const auto epsilon = number(1)/number(1<<8);
            const auto test = p.x*p.x + p.y*p.y;
            const auto test2 = p.x*p.x + p.y*p.y;
            return microtess::math::sqrt_cpu<number>(p.x*p.x + p.y*p.y, epsilon);
        }

        static auto build_quadrilateral(const vertex &a, const vertex &b,
                                 const number &stroke_width)-> poly_4 {
            poly_4 result;
            comp_parallel_ray(a, b, result.left, result.top, stroke_width);
            vertex dir=a-result.left;
            result.right=b+dir;
            result.bottom=a+dir;
            return result;
        }

        static auto finite_segment_intersection_test(const vertex &a, const vertex &b,
                                         const vertex &c, const vertex &d,
                                         vertex & intersection,
                                         number &alpha, number &alpha1) -> intersection_status {
            // this procedure will find proper and improper(touches) intersections,
            // but no overlaps, since overlaps induce parallel classification, this
            // would have to be resolved outside
            if(a==b || c==d)
                return intersection_status::degenerate_line;
            auto ab = b - a;
            auto cd = d - c;
            auto dem = ab.x * cd.y - ab.y * cd.x;
            auto abs_dem = dem<0?-dem:dem;

            // parallel lines
            if (abs_dem <= number(1))
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

        static bool rays_intersect(const vertex &a, const vertex &b,
                const vertex &c, const vertex &d, vertex &intersection) {
            // given, that infinite lines that pass through segments (a,b) and (c,d)
            // are not parallel, compute intersection.
            vertex s1 = b - a;
            vertex s2 = d - c;
            vertex dc = a - c;
            // we reduce the precision with multiplications to avoid extreme overflow
            number det = -(s2.x * s1.y) + (s1.x * s2.y);
            //number s = -(s1.y * dc.x) + (s1.x * dc.y);
            number t = (s2.x * dc.y) - (s2.y * dc.x);
            if (det!=0) intersection = a + (s1*t)/det;
            return det!=0;
        }

        static void comp_parallel_ray(const vertex &pt0,
                               const vertex &pt1,
                               vertex &pt_out_0,
                               vertex &pt_out_1,
                               number stroke) {
            vertex vec = pt1==pt0 ? vertex{1,0} : pt1 - pt0;
            vertex normal = {vec.y, -vec.x};
            number length = norm(normal);
            vertex dir = length ? (normal*stroke)/length : normal*stroke;
            pt_out_0 = pt0 + dir;
            pt_out_1 = pt1 + dir;
        }

        static auto resolve_left_right_walls_intersections(const poly_4 &a, const poly_4 &b)
                                                    -> poly_inter_result {
            poly_inter_result result{};
            vertex result_inter;
            number alpha;
            // left walls
            auto status= finite_segment_intersection_test(a.left, a.top,
                                               b.left, b.top, result_inter, alpha, alpha);
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

        static int classify_point(const vertex &point, const vertex &a, const vertex &b) {
            // todo: make it super robust with boosting
            // Use the sign of the determinant of vectors (AB,AM), where M(X,Y)
            // is the query point:
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

        static edge compute_distanced_tangent_at_joint(const vertex &a, const vertex &b,
                                                       const vertex &c, number mag) {
            edge result;
            vertex tangent1 = b-a, tangent2 = c-b;
            vertex perp_1 = {tangent1.y, -tangent1.x}, perp_2 = {tangent2.y, -tangent2.x};
            vertex pre_normal_join= perp_1+perp_2;
            if(pre_normal_join==vertex{0,0}) {
                pre_normal_join=c-a;
                mag= mag<0 ? -mag : mag;
            }
            const number length= norm(pre_normal_join);
            const vertex direction=length ? (pre_normal_join*mag)/length : (pre_normal_join*mag);
            result.a = b + direction;
            result.b = result.a + vertex{direction.y, -direction.x};
            return result;
        }

        static void compute_arc(const vertex &from, const vertex &root, const vertex &to,
                         const number &radius, const number &max_distance_squared,
                         const index & root_index,
                         container_output_vertices &output_vertices,
                         container_output_indices &output_indices,
                         container_output_boundary *boundary_buffer) {
            // this procedure will generate points on a half circle adaptively
            const auto dir= to-from;
            bool done= (dir.x*dir.x + dir.y*dir.y)<=max_distance_squared;
            if(done) return;
            const auto e= compute_distanced_tangent_at_joint(from, root, to, radius);
            bool not_in_cone_precision_issues= classify_point(e.a, root, from)>=0 ||
                    classify_point(e.a, root, to)<=0;
            if(not_in_cone_precision_issues) return; // in case, the bisecting point was not in cone
            compute_arc(from, root, e.a, radius, max_distance_squared, root_index,
                        output_vertices, output_indices, boundary_buffer);
            output_vertices.push_back(e.a); output_indices.push_back(output_vertices.size()-1);
            b2_(boundary_buffer, output_indices);
            output_indices.push_back(root_index);
            b1_(boundary_buffer, output_indices);
            compute_arc(e.a, root, to, radius, max_distance_squared, root_index,
                        output_vertices, output_indices, boundary_buffer);
        }

        static void apply_cap(const stroke_cap &cap,
                    bool is_start, const vertex &root,
                    const index &a_index, const index &b_index,
                  const number &radius,
                  container_output_vertices &output_vertices,
                  container_output_indices &output_indices,
                  container_output_boundary *boundary_buffer) {
            // create a cap to the left of edge (a, b)
#define b1 b1_(boundary_buffer, output_indices)
#define b2 b2_(boundary_buffer, output_indices)
#define b3 b3_(boundary_buffer, output_indices)

            const auto a= output_vertices[a_index];
            const auto b= output_vertices[b_index];
            switch (cap) {
                case stroke_cap::butt:
                    // do nothing
                    break;
                case stroke_cap::round:
                {
                    output_vertices.push_back(root); const index root_index= output_vertices.size()-1;
                    output_indices.push_back(a_index); b1;
                    output_indices.push_back(a_index); b1;
                    output_indices.push_back(root_index); b1;
                    compute_arc(a, root, b, radius, radius*number(4),
                                root_index, output_vertices, output_indices, boundary_buffer);
                    output_indices.push_back(b_index); b2;
                    output_indices.push_back(root_index); b1;
//                    output_indices.push_back(a_index); // close it
                    break;
                }
                case stroke_cap::square:
                {
                    const auto dir=b-a;
                    const auto n=vertex{dir.y, -dir.x};
                    const auto dir2 = (n*radius)/norm(n);
                    output_vertices.push_back(a+dir2); const auto ext_a_index= output_vertices.size()-1;
                    output_vertices.push_back(b+dir2); const auto ext_b_index= output_vertices.size()-1;
                    output_indices.push_back(ext_a_index); b1;
                    output_indices.push_back(ext_a_index); b1;
                    output_indices.push_back(ext_b_index); b1;
                    output_indices.push_back(a_index); b3;
                    output_indices.push_back(b_index); b2;
                    break;
                }
            }
#undef b1
#undef b2
#undef b3
        }

        static void apply_line_join(const stroke_line_join &line_join, const index &first_index,
                             const index &join_index, const index &last_index,
                             const number &join_radius, const number &miter_limit,
                             container_output_vertices &output_vertices,
                             container_output_indices &output_indices,
                             container_output_boundary *boundary_buffer= nullptr) {
            // insert vertices strictly between the start and last vertex of the join
#define b1 b1_(boundary_buffer, output_indices)
#define b2 b2_(boundary_buffer, output_indices)

            const auto first_vertex = output_vertices[first_index];
            const auto join_vertex = output_vertices[join_index];
            const auto last_vertex = output_vertices[last_index];
            switch (line_join) {
                case stroke_line_join::none:
                {
                    output_indices.push_back(join_index); b1;
                    break;
                }
                case stroke_line_join::miter:
                case stroke_line_join::miter_clip:
                {
                    // 1. find the clipping point
                    // 2. find the clipping line
                    // 3.
                    const number miter_threshold = miter_limit*join_radius;
                    const edge clip_ray=
                            compute_distanced_tangent_at_joint(first_vertex, join_vertex, last_vertex,
                                                               miter_threshold);
                    edge a_ray = {first_vertex, first_vertex+(join_vertex-first_vertex).orthogonalLeft()};
                    edge b_ray = {last_vertex, last_vertex+(join_vertex-last_vertex).orthogonalLeft()};
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
                        output_vertices.push_back(intersection); output_indices.push_back(output_vertices.size()-1); b2;
                        output_indices.push_back(join_index); b1;
                    } else if(line_join==stroke_line_join::miter_clip) {
                        // no found intersection inside the half clip space, so we use
                        // the two clipped points calculated
                        output_vertices.push_back(a_ray.b); output_indices.push_back(output_vertices.size()-1); b2;
                        output_indices.push_back(join_index); b1;
                        output_vertices.push_back(b_ray.b); output_indices.push_back(output_vertices.size()-1); b2;
                        output_indices.push_back(join_index); b1;
                    } else {
                        // else regular miter falls back to bevel (no vertices) if no intersection
                    }
                    break;
                }
                case stroke_line_join::round:
                {
                    compute_arc(first_vertex, join_vertex, last_vertex,
                                join_radius, join_radius*number(4),
                                join_index, output_vertices, output_indices, boundary_buffer);
                    break;
                }
                case stroke_line_join::bevel:
                    // bevel, add no point in between
                    break;
            }
#undef b1
#undef b2
        }

        static void b1_(container_output_boundary *boundary_buffer,
                 const container_output_indices &output_indices){
            if(boundary_buffer && output_indices.size()>=3)
                boundary_buffer->push_back(triangles::create_boundary_info(
                        false, false, false));
        }
        static void b2_(container_output_boundary *boundary_buffer,
                 const container_output_indices &output_indices){
            if(boundary_buffer && output_indices.size()>=3)
                boundary_buffer->push_back(triangles::create_boundary_info(
                        false, false, true));
        }
        static void b3_(container_output_boundary *boundary_buffer,
                 const container_output_indices &output_indices){
            if(boundary_buffer && output_indices.size()>=3)
                boundary_buffer->push_back(triangles::create_boundary_info(
                        true, false, true));
        }
        static void reinforce_(container_output_boundary *boundary_buffer,
                               container_output_indices &output_indices){
            if(output_indices.size()) {
                output_indices.push_back(output_indices.back()); b1_(boundary_buffer, output_indices);
            }
        }
    };
}
