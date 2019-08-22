#pragma once

namespace triangles {
    enum class TrianglesIndices {
        TRIANGLES,
        TRIANGLES_WITH_BOUNDARY,
        TRIANGLES_STRIP,
        TRIANGLES_FAN
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

    template <typename T>
    bool classifyTopLeftEdge(const bool CCW,
                                   const T & p0x, const T & p0y,
                                   const T & p1x, const T & p1y
                                   ) {
        bool res;

        if(CCW)
            res = (p1y - p0y)>=0 || (p1y==p0y && (p1x - p0x)<=0);
        else
            res = (p0y - p1y)>=0 || (p1y==p0y && (p0x - p1x)<=0);

        return res;
    }

    template <typename T>
    top_left_t classifyTopLeftEdges(const bool CCW,
                        const T & p0x, const T & p0y,
                        const T & p1x, const T & p1y,
                        const T & p2x, const T & p2y
                        ) {
        top_left_t res;

        res.first = classifyTopLeftEdge(CCW, p0x, p0y, p1x, p1y);
        res.second = classifyTopLeftEdge(CCW, p1x, p1y, p2x, p2y);
        res.third = classifyTopLeftEdge(CCW, p2x, p2y, p0x, p0y);

        return res;
    }

    using boundary_info = unsigned char;

    bool classify_boundary_info(const boundary_info &info,
                                unsigned int edge_index) {
        switch (edge_index) {
            case 0:
                return (info & 0b10000000)>>7;
            case 1:
                return (info & 0b01000000)>>6;
            case 2:
                return (info & 0b00100000)>>5;
            default:
                return false;
        }

    }

    boundary_info create_boundary_info(bool first,
                                       bool second,
                                       bool third) {

        boundary_info zero = 0b00000000;
        boundary_info result = zero;

        result |= first     ? 0b10000000 : zero;
        result |= second    ? 0b01000000 : zero;
        result |= third     ? 0b00100000 : zero;

        return result;
    }

};
