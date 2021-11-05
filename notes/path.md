#include <microgl/tesselation/stroke_tessellation.h>
#include <microgl/vertex2.h>

namespace tessellation {
#define min_(a, b) ((a)<(b) ? (a) : (b))
#define max_(a, b) ((a)>(b) ? (a) : (b))

    uint32_t sqrt_64(uint64_t a_nInput) {
        uint64_t op  = a_nInput;
        uint64_t res = 0;
        uint64_t one = uint64_t(1) << 62; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


        // "one" starts at the highest power of four <= than the argument.
        while (one > op)
        {
            one >>= 2;
        }

        while (one != 0)
        {
            if (op >= res + one)
            {
                op = op - (res + one);
                res = res + ( one<<1);
            }
            res >>= 1;
            one >>= 2;
        }
        return res;
    }

    path_tessellation::path_tessellation(bool DEBUG) {

    }

    void path_tessellation::compute(index stroke_size,
                                    const microgl::vec2_f *$pts,
                                    const index size,
                                    array_container<index> &indices_buffer_tessellation,
                                    array_container<vec2_32i> &output_vertices_buffer_tessellation,
                                    array_container<triangles::boundary_info> * boundary_buffer,
                                    const microgl::triangles::TrianglesIndices &requested) {

    }

    unsigned int length_(const vec2_32i &vec) {
        return sqrt_64(int64_t(vec.x)*vec.x + int64_t(vec.y)*vec.y);
    }

    bool non_parallel_rays_intersect(
                    const vec2_32i &a,
                    const vec2_32i &b,
                    const vec2_32i &c,
                    const vec2_32i &d,
                    vec2_32i &intersection,
                    const precision precision
                    ) {
        vec2_32i s1 = b - a;
        vec2_32i s2 = d - c;
        vec2_32i dc = a - c;

        // we reduce the precision with multiplications to avoid extreme overflow
        int64_t det = -((int64_t(s2.x) * int64_t(s1.y))>>precision) + ((int64_t(s1.x) * int64_t(s2.y))>>precision);
        int64_t s = -((int64_t(s1.y) * int64_t(dc.x))>>precision) + ((int64_t(s1.x) * int64_t(dc.y))>>precision);
        int64_t t = ((int64_t(s2.x) * int64_t(dc.y))>>precision) - ((int64_t(s2.y) * int64_t(dc.x))>>precision);

//        if (s.isRegular() && t.isRegular() && s >= 0 && s <= 1 && t >= 0 && t <= 1) {

        if(det) {
            // try this later
            // we mix 64 bit precision to avoid overflow
            int64_t dir_x = (int64_t (s1.x)*t)/det;
            int64_t dir_y = (int64_t (s1.y)*t)/det;
            vec2_32i dir{dir_x, dir_y};
            intersection = a + dir;

            return true;
        }

        return false;
    }

    void comp_parallel_ray(const vec2_32i &pt0,
                           const vec2_32i &pt1,
                           vec2_32i &pt_out_0,
                           vec2_32i &pt_out_1,
                           int stroke
    ) {
        vec2_32i vec = pt1 - pt0;
        vec2_32i normal = {-vec.y, vec.x};
        uint32_t length = length_(normal);

        int dir_x = (int64_t(normal.x)*stroke)/length;
        int dir_y = (int64_t(normal.y)*stroke)/length;
        vec2_32i dir{dir_x, dir_y};

        pt_out_0 = pt0 - dir;
        pt_out_1 = pt1 - dir;
    }

    void merge_intersection(
            const vec2_32i &a,
            const vec2_32i &b,
            const vec2_32i &c,
            const vec2_32i &d,
            vec2_32i &merge_out,
            const precision precision
    ) {
        bool co_linear = !non_parallel_rays_intersect(
                a, b, c, d,
                merge_out,
                precision);

        if(co_linear)
            merge_out = b;
    }

    template <typename number>
    void path_tessellation<number>::compute(int stroke_size,
                                    bool closePath,
                                    const gravity gravity,
                                    const microgl::vec2_32i *points,
                                    const index size,
                                    const precision precision,
                                    dynamic_array<index> &indices_buffer_tessellation,
                                    dynamic_array<vertex> &output_vertices_buffer_tessellation,
                                    dynamic_array<microgl::triangles::boundary_info> * boundary_buffer,
                                    const microgl::triangles::TrianglesIndices &requested
                                   ) {

        bool with_boundary =
                requested==microgl::triangles::TrianglesIndices::TRIANGLES_STRIP_WITH_BOUNDARY;

        microgl::triangles::boundary_info common_info_1 =
                microgl::triangles::create_boundary_info(false, false, true);
        microgl::triangles::boundary_info common_info_2 =
                microgl::triangles::create_boundary_info(true, false, true);

        switch (gravity) {
            case gravity::center:
                stroke_size = max_(1, stroke_size>>1);
                break;
            case gravity::inward:
                stroke_size = -stroke_size;
                break;
            case gravity::outward:
                stroke_size = stroke_size;
                break;
        }

        index idx = 0;
        vertex p0_out_current, p1_out_current;
        vertex p0_out_next, p1_out_next;
        vec2_32i merge_out, merge_in;

        comp_parallel_ray(points[0],
                          points[1],
                          p0_out_current,
                          p1_out_current,
                          stroke_size);

        switch (gravity) {
            case gravity::center:
                merge_in = p0_out_current;
                merge_out = points[0] - (p0_out_current - points[0]);
                break;
            case gravity::inward:
                merge_in = points[0];
                merge_out = p0_out_current;
                break;
            case gravity::outward:
                merge_out = points[0];
                merge_in = p0_out_current;
                break;
        }

        output_vertices_buffer_tessellation.push_back(merge_out);
        output_vertices_buffer_tessellation.push_back(merge_in);
        indices_buffer_tessellation.push_back(idx++);
        indices_buffer_tessellation.push_back(idx++);

        bool even=true;
        for (index ix = 1; ix < size - 1; ++ix) {

            comp_parallel_ray(points[ix],
                              points[ix+1],
                              p0_out_next,
                              p1_out_next,
                              stroke_size);

            merge_intersection(
                    p0_out_current,
                    p1_out_current,
                    p0_out_next,
                    p1_out_next,
                    merge_out,
                    precision
            );

            switch (gravity) {
                case gravity::center:
                    merge_in = merge_out;
                    merge_out = points[ix] - (merge_out - points[ix]);
                    break;
                case gravity::inward:
                    merge_in = points[ix];
                    break;
                case gravity::outward:
                    merge_in = merge_out;
                    merge_out = points[ix];
                    break;
            }

            output_vertices_buffer_tessellation.push_back(merge_out);
            output_vertices_buffer_tessellation.push_back(merge_in);
            indices_buffer_tessellation.push_back(idx++);
            indices_buffer_tessellation.push_back(idx++);

            if(with_boundary) {
                boundary_buffer->push_back(common_info_1);
                boundary_buffer->push_back(common_info_1);
            }

            p0_out_current = p0_out_next;
            p1_out_current = p1_out_next;
            even = !even;
        }

        // edge cases
        if(!closePath) {
            comp_parallel_ray(points[size-2],
                              points[size-1],
                              p0_out_current,
                              p1_out_current,
                              stroke_size
            );


            switch (gravity) {
                case gravity::center:
                    merge_in = p1_out_current;
                    merge_out = points[size-1] - (p1_out_current - points[size-1]);
                    break;
                case gravity::inward:
                    merge_in = points[size-1];
                    merge_out = p1_out_current;
                    break;
                case gravity::outward:
                    merge_in = p1_out_current;
                    merge_out = points[size-1];
                    break;
            }

            output_vertices_buffer_tessellation.push_back(merge_out);
            output_vertices_buffer_tessellation.push_back(merge_in);
            indices_buffer_tessellation.push_back(idx++);
            indices_buffer_tessellation.push_back(idx++);

            // fix the first triangle boundary
            if(with_boundary) {
                boundary_buffer->push_back(common_info_1);
                boundary_buffer->push_back(common_info_2);
                (*boundary_buffer)[0] = common_info_2;
            }

        }
        else {
            // last segment to first segment,
            // patch first two points.
            comp_parallel_ray(points[size-1],
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
                    merge_out,
                    precision
            );

            switch (gravity) {
                case gravity::center:
                    merge_in = merge_out;
                    merge_out = points[0] - (merge_out - points[0]);
                    break;
                case gravity::inward:
                    merge_in = points[0];
                    break;
                case gravity::outward:
                    merge_in = merge_out;
                    merge_out = points[0];
                    break;
            }

            output_vertices_buffer_tessellation[0] = merge_out;
            output_vertices_buffer_tessellation[1] = merge_in;

            // before last and last segment,
            // create last two triangles that connect
            // the last point to the first.
            comp_parallel_ray(points[size-2],
                              points[size-1],
                              p0_out_current,
                              p1_out_current,
                              stroke_size);

            comp_parallel_ray(points[size-1],
                              points[0],
                              p0_out_next,
                              p1_out_next,
                              stroke_size);

            merge_intersection(
                    p0_out_current,
                    p1_out_current,
                    p0_out_next,
                    p1_out_next,
                    merge_out,
                    precision
            );

            switch (gravity) {
                case gravity::center:
                    merge_in = merge_out;
                    merge_out = points[size-1] - (merge_out - points[size-1]);
                    break;
                case gravity::inward:
                    merge_in = points[size-1];
                    break;
                case gravity::outward:
                    merge_in = merge_out;
                    merge_out = points[size-1];
                    break;
            }

            output_vertices_buffer_tessellation.push_back(merge_out);
            output_vertices_buffer_tessellation.push_back(merge_in);
            indices_buffer_tessellation.push_back(idx++);
            indices_buffer_tessellation.push_back(idx++);

            output_vertices_buffer_tessellation.push_back(output_vertices_buffer_tessellation[0]);
            output_vertices_buffer_tessellation.push_back(output_vertices_buffer_tessellation[1]);
            indices_buffer_tessellation.push_back(idx++);
            indices_buffer_tessellation.push_back(idx++);

            if(with_boundary) {
                boundary_buffer->push_back(common_info_1);
                boundary_buffer->push_back(common_info_1);
                boundary_buffer->push_back(common_info_1);
                boundary_buffer->push_back(common_info_1);
            }

        }

    }

}
