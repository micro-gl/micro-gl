#include <microgl/tesselation/FanTriangulation.h>

namespace tessellation {

    FanTriangulation::FanTriangulation(bool DEBUG) {

    }

    index *FanTriangulation::compute(vec2_32i *$pts,
                                     index size,
                                     index *indices_buffer_triangulation,
                                     index indices_buffer_size,
                                     const triangles::TrianglesIndices &requested) {

        if(requested==triangles::TrianglesIndices::TRIANGLES_FAN) {
            if(required_indices_size(size, requested) > indices_buffer_size)
                throw std::runtime_error("size of the indices buffer is "
                                         "not enough for TRIANGLES_FAN !!!");
        }
        else if(requested==triangles::TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY) {
            if(required_indices_size(size, requested) > indices_buffer_size)
                throw std::runtime_error("size of the indices buffer is not enough "
                                         "for TRIANGLES_FAN_WITH_BOUNDARY !!!");
        }
        else
            throw std::runtime_error("only TRIANGLES_FAN and TRIANGLES_FAN_WITH_BOUNDARY "
                                     "are supported !!!");

        bool requested_triangles_with_boundary =
                requested==triangles::TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY;
        auto *indices = indices_buffer_triangulation;
        index count = requested_triangles_with_boundary ? 2*size - 2 : size;

        if(requested_triangles_with_boundary) {
            const index num_triangles = size-2;

            for (index ix = 0; ix < num_triangles; ++ix) {
                if(ix==0) {
                    indices[0] = 0;
                    indices[1] = 1;
                }

                bool aa_first_edge = ix==0;
                bool aa_second_edge = true;
                bool aa_third_edge = ix==num_triangles-1;

                triangles::boundary_info aa_info =
                        triangles::create_boundary_info(aa_first_edge,
                                                        aa_second_edge,
                                                        aa_third_edge);

                index ind = (ix+1)*2;
                indices[ind]     = ix + 2;
                indices[ind + 1] = aa_info;
            }
        }
        else {
            for (index ix = 0; ix < count; ++ix) {
                indices[ix] = ix;
            }
        }

        return indices;
    }

    index *
    FanTriangulation::compute(vec2_f *$pts,
                              index size,
                              index *indices_buffer_triangulation,
                              index indices_buffer_size,
                              const triangles::TrianglesIndices &requested) {
        // I could have made a template for point types and
        // conserve stack memory, but the hell with it for now
        vec2_32i vertices_int[size];

        for (index ix = 0; ix < size; ++ix) {
            vertices_int[ix] = $pts[ix]<<5;
        }

        return compute(vertices_int, size,
                       indices_buffer_triangulation,
                       indices_buffer_size, requested);
    }

    index FanTriangulation::required_indices_size(const index polygon_size,
                                                  const triangles::TrianglesIndices &requested) {
        switch (requested) {
            case triangles::TrianglesIndices::TRIANGLES_FAN:
                return polygon_size;
            case triangles::TrianglesIndices::TRIANGLES_FAN_WITH_BOUNDARY:
                return (polygon_size - 1)*2;
            default:
                return 0;
        }
    }

}