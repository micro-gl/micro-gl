#include <microgl/tesselation/PathTessellation.h>

namespace tessellation {

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

    PathTessellation::PathTessellation(bool DEBUG) {

    }

    void PathTessellation::compute(index stroke_size,
                                   const microgl::vec2_f *$pts,
                                   const index size,
                                   array_container<index> &indices_buffer_tessellation,
                                   array_container<vec2_32i> &output_vertices_buffer_tessellation,
                                   const microgl::triangles::TrianglesIndices &requested) {

    }

    unsigned int length_(const vec2_32i &vec) {
        return sqrt_64(int64_t(vec.x)*vec.x + int64_t(vec.y)*vec.y);
    }


    bool non_parallel_rays_intersect(const vec2_32i &a,
                   const vec2_32i &b,
                   const vec2_32i &c,
                   const vec2_32i &d,
                   vec2_32i &intersection,
                   const precision precision
                   ) {
        vec2_32i s1 = b - a;
        vec2_32i s2 = d - c;
        vec2_32i dc = a - c;
//
//        int64_t det = -int64_t(s2.x) * int64_t(s1.y) + int64_t(s1.x) * int64_t(s2.y);
//        int64_t s = -int64_t(s1.y) * int64_t(dc.x) + int64_t(s1.x) * int64_t(dc.y);
//        int64_t t = int64_t(s2.x) * int64_t(dc.y) - int64_t(s2.y) * int64_t(dc.x);


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
                           index stroke,
                           const precision precision
    ) {
        vec2_32i vec = pt1 - pt0;
        vec2_32i normal = {-vec.y, vec.x};
        uint32_t length = length_(normal);

        int dir_x = (int64_t(normal.x)*stroke)/length;
        int dir_y = (int64_t(normal.y)*stroke)/length;
        vec2_32i dir{dir_x, dir_y};

//        vec_out = vec - ((normal*stroke)/length);
        pt_out_0 = pt0 - dir;
        pt_out_1 = pt1 - dir;
    }

    void merge_intersection(
            const vec2_32i &a,
            const vec2_32i &b,
            const vec2_32i &c,
            const vec2_32i &d,
            vec2_32i &merge_out,
            index stroke_size,
            const precision precision
    ) {
        bool co_linear = !non_parallel_rays_intersect(
                a, b, c, d,
                merge_out,
                precision);

        if(co_linear)
            merge_out = b;
    }

    void PathTessellation::compute(index stroke_size,
                                   const microgl::vec2_32i *points,
                                   const index size,
                                   const precision precision,
                                   array_container<index> &indices_buffer_tessellation,
                                   array_container<vec2_32i> &output_vertices_buffer_tessellation,
                                   const microgl::triangles::TrianglesIndices &requested,
                                   bool closePath) {

        index idx = 0;
        vec2_32i vec_0_out_current, vec_1_out_current;

        vec2_32i merge_out, merge_in;

        comp_parallel_ray(points[0],
                          points[1],
                          vec_0_out_current,
                          vec_1_out_current,
                          stroke_size,
                          precision);

        merge_out = vec_0_out_current;
        merge_in = points[0] - (vec_0_out_current - points[0]);

        output_vertices_buffer_tessellation.push_back(merge_out);
        output_vertices_buffer_tessellation.push_back(merge_in);
        indices_buffer_tessellation.push_back(idx++);
        indices_buffer_tessellation.push_back(idx++);

        for (index ix = 1; ix < size - 1; ++ix) {
            vec2_32i vec_1_out_next, vec_0_out_next;

            comp_parallel_ray(points[ix],
                              points[ix+1],
                              vec_0_out_next,
                              vec_1_out_next,
                              stroke_size,
                              precision);

            merge_intersection(
                    vec_0_out_current,
                    vec_1_out_current,
                    vec_0_out_next,
                    vec_1_out_next,
                    merge_out,
                    stroke_size,
                    precision
            );

            merge_in = points[ix] - (merge_out - points[ix]);

            output_vertices_buffer_tessellation.push_back(merge_out);
            output_vertices_buffer_tessellation.push_back(merge_in);
            indices_buffer_tessellation.push_back(idx++);
            indices_buffer_tessellation.push_back(idx++);

            vec_0_out_current = vec_0_out_next;
            vec_1_out_current = vec_1_out_next;
        }

        comp_parallel_ray(points[size-2],
                          points[size-1],
                          vec_0_out_current,
                          vec_1_out_current,
                          stroke_size,
                          precision
                          );

        merge_out = vec_1_out_current;
        merge_in = points[size-1] - (vec_1_out_current - points[size-1]);

        output_vertices_buffer_tessellation.push_back(merge_out);
        output_vertices_buffer_tessellation.push_back(merge_in);
        indices_buffer_tessellation.push_back(idx++);
        indices_buffer_tessellation.push_back(idx++);

    }

    /*
    void PathTessellation::compute(index stroke_size,
                                   const microgl::vec2_32i *points,
                                   const index size,
                                   array_container<index> &indices_buffer_tessellation,
                                   array_container<vec2_32i> &output_vertices_buffer_tessellation,
                                   const microgl::triangles::TrianglesIndices &requested,
                                   bool closePath) {

        index idx = 0;
        vec2_32i vec_1_out_current;
        vec2_32i vec_1_out_next;
        vec2_32i merge_out, merge_in;

        comp_normal(points[0],
                points[1],
                    vec_1_out_current
        );

        merge_intersection(
                points[0],
                vec_1_out_current,
                vec_1_out_current,
                merge_out,
                merge_in,
                stroke_size
        );

        output_vertices_buffer_tessellation.push_back(merge_out);
        output_vertices_buffer_tessellation.push_back(merge_in);
        indices_buffer_tessellation.push_back(idx++);
        indices_buffer_tessellation.push_back(idx++);

        for (index ix = 1; ix < size - 1; ++ix) {

            comp_normal(points[ix],
                    points[ix+1],
                        vec_1_out_next
            );

            merge_intersection(
                    points[ix],
                    vec_1_out_current,
                    vec_1_out_next,
                    merge_out,
                    merge_in,
                    stroke_size
                    );

            output_vertices_buffer_tessellation.push_back(merge_out);
            output_vertices_buffer_tessellation.push_back(merge_in);
            indices_buffer_tessellation.push_back(idx++);
            indices_buffer_tessellation.push_back(idx++);

            vec_1_out_current = vec_1_out_next;
        }

        comp_normal(points[size-2],
                    points[size-1],
                    vec_1_out_current
        );

        merge_intersection(
                points[size-1],
                vec_1_out_current,
                vec_1_out_current,
                merge_out,
                merge_in,
                stroke_size
        );

        output_vertices_buffer_tessellation.push_back(merge_out);
        output_vertices_buffer_tessellation.push_back(merge_in);
        indices_buffer_tessellation.push_back(idx++);
        indices_buffer_tessellation.push_back(idx++);

    }

     */
}
