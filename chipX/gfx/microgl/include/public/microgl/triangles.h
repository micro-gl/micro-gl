#pragma once


namespace microgl {
    namespace triangles {

        enum class indices {
            TRIANGLES,
            // every fourth element is a boundary info element
            TRIANGLES_WITH_BOUNDARY,
            // triangle fan, first vertex is the main vertex to which all triangles
            // are expended into
            TRIANGLES_FAN,
            // every second element is a boundary info element
            // starting from the fourth element
            TRIANGLES_FAN_WITH_BOUNDARY,
            // triangle strip
            TRIANGLES_STRIP,
            // triangle strip with boundary info
            TRIANGLES_STRIP_WITH_BOUNDARY,
        };

        enum class TriangleEdgeType {
            Top,
            Left,
            Right,
        };

        enum class orientation {
            cw, ccw
        };

        enum class face_culling {
            cw, ccw, none
        };

        struct top_left_t {
            bool first = false, second = false, third = false;
        };

        template<typename T>
        bool classifyTopLeftEdge(const bool CCW,
                                 const T &p0x, const T &p0y,
                                 const T &p1x, const T &p1y
        ) {
            bool res;

            if (CCW)
                res = (p1y>p0y) || (p1y==p0y && (p1x<=p0x));
            else
                res = (p0y>p1y) || (p1y==p0y && (p0x<=p1x));

            return res;
        }

        template<typename T>
        top_left_t classifyTopLeftEdges(const bool CCW,
                                        const T &p0x, const T &p0y,
                                        const T &p1x, const T &p1y,
                                        const T &p2x, const T &p2y
        ) {
            top_left_t res;

            res.first = classifyTopLeftEdge(CCW, p0x, p0y, p1x, p1y);
            res.second = classifyTopLeftEdge(CCW, p1x, p1y, p2x, p2y);
            res.third = classifyTopLeftEdge(CCW, p2x, p2y, p0x, p0y);

            return res;
        }

        using boundary_info = unsigned char;
        using index = unsigned int;

        bool classify_boundary_info(const boundary_info &info,
                                    unsigned int edge_index);
        boundary_info create_boundary_info(bool first,
                                           bool second,
                                           bool third);

        template<typename iterator_callback>
        void iterate_triangles(const index *indices,
                               const index &size,
                               const enum triangles::indices &type,
                               const iterator_callback && callback) {
#define IND(a) ((indices) ? indices[(a)] : (a))
            switch (type) {
                case indices::TRIANGLES:
                case indices::TRIANGLES_WITH_BOUNDARY:
                    for (index ix = 0, idx=0; ix < size; ix+=3,idx++)
                        callback(idx, IND(ix + 0), IND(ix + 1), IND(ix + 2));
                    break;
                case indices::TRIANGLES_FAN:
                case indices::TRIANGLES_FAN_WITH_BOUNDARY:
                    for (index ix = 1; ix < size-1; ++ix)
                        callback(ix-1, IND(0), IND(ix), IND(ix + 1));
                    break;
                case indices::TRIANGLES_STRIP:
                case indices::TRIANGLES_STRIP_WITH_BOUNDARY:
                {
                    bool even = true;
                    for (index ix = 0; ix < size-2; ++ix) {
                        // we alternate order inorder to preserve CCW or CW,
                        index first_index = even ?  IND(ix + 0) : IND(ix + 2);
                        index second_index = IND(ix + 1);
                        index third_index = even ?  IND(ix + 2) : IND(ix + 0);
                        callback(ix, first_index, second_index, third_index);
                        even = !even;
                    }
                    break;
                }
            }
#undef IND
        }

    };
}