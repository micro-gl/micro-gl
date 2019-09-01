#include <microgl/tesselation/PathTessellation.h>

namespace tessellation {
    uint32_t sqrt_int(uint32_t a_nInput) {
        uint32_t op  = a_nInput;
        uint32_t res = 0;
        uint32_t one = 1 << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


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

        int det = -s2.x * s1.y + s1.x * s2.y;
        int s = (-s1.y * dc.x + s1.x * dc.y);
        int t = ( s2.x * dc.y - s2.y * dc.x);

//        if (s.isRegular() && t.isRegular() && s >= 0 && s <= 1 && t >= 0 && t <= 1) {

        if(det) {
            // try this later
            intersection = a + ((s1 * t))/det;

            return true;
        }

        return false;
    }

    int length_(const vec2_32i &vec) {
        return sqrt_int(vec.x*vec.x + vec.y*vec.y);
    }

    void comp_normal(const vec2_32i &pt0,
                     const vec2_32i &pt1,
                     vec2_32i &direction_out
    ) {
        vec2_32i vec = pt1 - pt0;
        vec2_32i normal = {-vec.y, vec.x};
        int length = length_(normal);

        direction_out = -(normal);///length;
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
        int length = length_(normal);

//        vec_out = vec - ((normal*stroke)/length);
        pt_out_0 = pt0 - ((normal*stroke))/length;
        pt_out_1 = pt1 - ((normal*stroke))/length;
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
