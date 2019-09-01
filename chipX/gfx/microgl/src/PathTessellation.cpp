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

    void merge_intersection(
                     const vec2_32i &intersection,
                     const vec2_32i &vec_1_out,
                     const vec2_32i &vec_2_out,
                     vec2_32i &merge_out,
                     vec2_32i &merge_in,
                     index stroke_size=10
    ) {

        vec2_32i out_vec = (vec_1_out + vec_2_out)/2;
        out_vec = (out_vec*stroke_size)/length_(out_vec);

        merge_out = intersection + out_vec;
        merge_in = intersection - out_vec;
    }

    void PathTessellation::compute(index stroke_size,
                                   const microgl::vec2_32i *points,
                                   const index size,
                                   array_container<index> &indices_buffer_tessellation,
                                   array_container<vec2_32i> &output_vertices_buffer_tessellation,
                                   const microgl::triangles::TrianglesIndices &requested,
                                   bool closePath) {

        index idx = 0;
        vec2_32i vec_1_out_current, vec_1_in_current,
                    vec_2_out_current, vec_2_in_current;
        vec2_32i vec_1_out_next, vec_1_in_next,
                    vec_2_out_next, vec_2_in_next;
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

//            vec_1_in_current = vec_1_in_next;
//            vec_2_out_current = vec_2_out_next;
//            vec_2_in_current = vec_2_in_next;

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

}
