#include <microgl/TriangleIndices.h>

namespace triangles {

    bool classify_boundary_info(const boundary_info &info, unsigned int edge_index) {
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

    boundary_info create_boundary_info(bool first, bool second, bool third) {

        boundary_info zero = 0b00000000;
        boundary_info result = zero;

        result |= first     ? 0b10000000 : zero;
        result |= second    ? 0b01000000 : zero;
        result |= third     ? 0b00100000 : zero;

        return result;
    }
}