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
        void stroke_tessellation<number>::comp_parallel_ray(
                const vertex &pt0,
                const vertex &pt1,
                vertex &pt_out_0,
                vertex &pt_out_1,
                number stroke
        ) {
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
            closePath=closePath||(points[0]==points[size-1]);
            bool with_boundary =boundary_buffer!= nullptr;
            output_indices_type=triangles::indices::TRIANGLES_STRIP;

            microgl::triangles::boundary_info common_info_1 =
                    microgl::triangles::create_boundary_info(false, false, true);
            microgl::triangles::boundary_info common_info_2 =
                    microgl::triangles::create_boundary_info(true, false, true);

            switch (gravity) {
                case stroke_gravity::center:
                    stroke_size = max_(number(1), stroke_size / number(2));
                    break;
                case stroke_gravity::inward:
                    stroke_size = -stroke_size;
                    break;
                case stroke_gravity::outward:
                    stroke_size = stroke_size;
                    break;
            }

            index start=output_vertices.size(), idx = 0;
            vertex p0_out_current, p1_out_current;
            vertex p0_out_next, p1_out_next;
            vertex merge_out, merge_in;

            comp_parallel_ray(points[0],
                              points[1],
                              p0_out_current,
                              p1_out_current,
                              stroke_size);

            switch (gravity) {
                case stroke_gravity::center:
                    merge_in = p0_out_current;
                    merge_out = points[0] - (p0_out_current - points[0]);
                    break;
                case stroke_gravity::inward:
                    merge_in = points[0];
                    merge_out = p0_out_current;
                    break;
                case stroke_gravity::outward:
                    merge_out = points[0];
                    merge_in = p0_out_current;
                    break;
            }

            output_vertices.push_back(merge_out);
            output_vertices.push_back(merge_in);
            output_indices.push_back(start + idx++);
            output_indices.push_back(start + idx++);

            bool even = true;
            for (index ix = 1; ix < size - 1; ++ix) {

                comp_parallel_ray(points[ix],
                                  points[ix + 1],
                                  p0_out_next,
                                  p1_out_next,
                                  stroke_size);

                merge_intersection(
                        p0_out_current,
                        p1_out_current,
                        p0_out_next,
                        p1_out_next,
                        merge_out
                );

                switch (gravity) {
                    case stroke_gravity::center:
                        merge_in = merge_out;
                        merge_out = points[ix] - (merge_out - points[ix]);
                        break;
                    case stroke_gravity::inward:
                        merge_in = points[ix];
                        break;
                    case stroke_gravity::outward:
                        merge_in = merge_out;
                        merge_out = points[ix];
                        break;
                }

                output_vertices.push_back(merge_out);
                output_vertices.push_back(merge_in);
                output_indices.push_back(start + idx++);
                output_indices.push_back(start + idx++);

                if (with_boundary) {
                    boundary_buffer->push_back(common_info_1);
                    boundary_buffer->push_back(common_info_1);
                }

                p0_out_current = p0_out_next;
                p1_out_current = p1_out_next;
                even = !even;
            }

            // now patch edge cases
            if (!closePath) {
                comp_parallel_ray(points[size - 2],
                                  points[size - 1],
                                  p0_out_current,
                                  p1_out_current,
                                  stroke_size
                );

                switch (gravity) {
                    case stroke_gravity::center:
                        merge_in = p1_out_current;
                        merge_out = points[size - 1] - (p1_out_current - points[size - 1]);
                        break;
                    case stroke_gravity::inward:
                        merge_in = points[size - 1];
                        merge_out = p1_out_current;
                        break;
                    case stroke_gravity::outward:
                        merge_in = p1_out_current;
                        merge_out = points[size - 1];
                        break;
                }

                output_vertices.push_back(merge_out);
                output_vertices.push_back(merge_in);
                output_indices.push_back(start + idx++);
                output_indices.push_back(start + idx++);

                // fix the first triangle boundary
                if (with_boundary) {
                    boundary_buffer->push_back(common_info_1);
                    boundary_buffer->push_back(common_info_2);
                    (*boundary_buffer)[0] = common_info_2;
                }

            } else {
                // last segment to first segment,
                // patch first two points.
                comp_parallel_ray(points[size - 1],
                                  points[0],
                                  p0_out_current,
                                  p1_out_current,
                                  stroke_size);

                comp_parallel_ray(points[0],
                                  points[1],
                                  p0_out_next,
                                  p1_out_next,
                                  stroke_size);

                merge_intersection(
                        p0_out_current,
                        p1_out_current,
                        p0_out_next,
                        p1_out_next,
                        merge_out
                );

                switch (gravity) {
                    case stroke_gravity::center:
                        merge_in = merge_out;
                        merge_out = points[0] - (merge_out - points[0]);
                        break;
                    case stroke_gravity::inward:
                        merge_in = points[0];
                        break;
                    case stroke_gravity::outward:
                        merge_in = merge_out;
                        merge_out = points[0];
                        break;
                }

                output_vertices[0] = merge_out;
                output_vertices[1] = merge_in;

                // before first and last segment,
                // create last two triangles that connect
                // the last point to the first.
                comp_parallel_ray(points[size - 2],
                                  points[size - 1],
                                  p0_out_current,
                                  p1_out_current,
                                  stroke_size);

                comp_parallel_ray(points[size - 1],
                                  points[0],
                                  p0_out_next,
                                  p1_out_next,
                                  stroke_size);

                merge_intersection(
                        p0_out_current,
                        p1_out_current,
                        p0_out_next,
                        p1_out_next,
                        merge_out
                );

                switch (gravity) {
                    case stroke_gravity::center:
                        merge_in = merge_out;
                        merge_out = points[size - 1] - (merge_out - points[size - 1]);
                        break;
                    case stroke_gravity::inward:
                        merge_in = points[size - 1];
                        break;
                    case stroke_gravity::outward:
                        merge_in = merge_out;
                        merge_out = points[size - 1];
                        break;
                }

                output_vertices.push_back(merge_out);
                output_vertices.push_back(merge_in);
                output_indices.push_back(start + idx++);
                output_indices.push_back(start + idx++);

                output_vertices.push_back(output_vertices[start + 0]);
                output_vertices.push_back(output_vertices[start + 1]);
                output_indices.push_back(start + idx++);
                output_indices.push_back(start + idx++);

                if (with_boundary) {
                    boundary_buffer->push_back(common_info_1);
                    boundary_buffer->push_back(common_info_1);
                    boundary_buffer->push_back(common_info_1);
                    boundary_buffer->push_back(common_info_1);
                }

            }

        }

    }

}