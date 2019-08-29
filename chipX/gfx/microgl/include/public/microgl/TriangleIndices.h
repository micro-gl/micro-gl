#pragma once


namespace microgl {
    namespace triangles {

        enum class TrianglesIndices {
            TRIANGLES,
            // every fourth element is a boundary info element
            TRIANGLES_WITH_BOUNDARY,
            TRIANGLES_FAN,
            // every second element is a boundary info element
            // starting from the fourth element
            TRIANGLES_FAN_WITH_BOUNDARY,
            TRIANGLES_STRIP,
        };

        enum class TriangleEdgeType {
            Top,
            Left,
            Right,
        };

        enum class TriangleOrientation {
            CW,
            CCW,
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
                res = (p1y - p0y) >= 0 || (p1y == p0y && (p1x - p0x) <= 0);
            else
                res = (p0y - p1y) >= 0 || (p1y == p0y && (p0x - p1x) <= 0);

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

        bool classify_boundary_info(const boundary_info &info,
                                    unsigned int edge_index);

        boundary_info create_boundary_info(bool first,
                                           bool second,
                                           bool third);

    };
}