/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

namespace microtess {
    namespace triangles {

        enum class indices {
            TRIANGLES,
            TRIANGLES_WITH_BOUNDARY,
            // triangle fan, first vertex is the main vertex to which all triangles
            // are expended into
            TRIANGLES_FAN,
            TRIANGLES_FAN_WITH_BOUNDARY,
            // triangle strip
            TRIANGLES_STRIP,
            TRIANGLES_STRIP_WITH_BOUNDARY,
        };

        enum class TriangleEdgeType { Top, Left, Right };
        enum class orientation { cw, ccw };
        enum class face_culling { cw, ccw, none };
        struct top_left_t { bool first = false, second = false, third = false; };

        template<typename number>
        bool classifyTopLeftEdge(const bool CCW,
                                 const number &p0x, const number &p0y,
                                 const number &p1x, const number &p1y) {
            bool res;
            if (CCW) res = (p1y>p0y) || (p1y==p0y && (p1x<=p0x));
            else res = (p0y>p1y) || (p1y==p0y && (p0x<=p1x));
            return res;
        }

        template<typename number>
        top_left_t classifyTopLeftEdges(const bool CCW,
                                        const number &p0x, const number &p0y,
                                        const number &p1x, const number &p1y,
                                        const number &p2x, const number &p2y) {
            top_left_t res;
            res.first = classifyTopLeftEdge<number>(CCW, p0x, p0y, p1x, p1y);
            res.second = classifyTopLeftEdge<number>(CCW, p1x, p1y, p2x, p2y);
            res.third = classifyTopLeftEdge<number>(CCW, p2x, p2y, p0x, p0y);
            return res;
        }

        using boundary_info = unsigned char;
        using index = unsigned int;

        static bool classify_boundary_info(const boundary_info &info, unsigned int edge_index) {
            switch (edge_index) {
                case 0: return (info & 0b10000000)>>7;
                case 1: return (info & 0b01000000)>>6;
                case 2: return (info & 0b00100000)>>5;
                default: return false;
            }
        }

        static inline boundary_info create_boundary_info(bool first, bool second, bool third) {
            boundary_info zero = 0b00000000;
            boundary_info result = zero;
            result |= first     ? 0b10000000 : zero;
            result |= second    ? 0b01000000 : zero;
            result |= third     ? 0b00100000 : zero;
            return result;
        }

        /**
         * Iterate triangles encoded in a list of indices
         * @tparam iterator_callback a callback struct or lambda
         * @param indices pointer to indices array
         * @param size size of indices list
         * @param type the type of triangles
         * @param callback the callback instance
         */
        template<typename iterator_callback>
        void iterate_triangles(const index *indices,
                               const index &size,
                               const enum triangles::indices &type,
                               const iterator_callback & callback) {
#define IND(a) ((indices) ? indices[(a)] : (a))
            switch (type) {
                case indices::TRIANGLES:
                case indices::TRIANGLES_WITH_BOUNDARY:
                    for (index ix = 0, idx=0; ix < size; ix+=3,idx++)
                        callback(idx, IND(ix + 0), IND(ix + 1), IND(ix + 2), 0, 1, 2);
                    break;
                case indices::TRIANGLES_FAN:
                case indices::TRIANGLES_FAN_WITH_BOUNDARY:
                    for (index ix = 1; ix < size-1; ++ix)
                        callback(ix-1, IND(0), IND(ix), IND(ix + 1), 0, 1, 2);
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
                        callback(ix, first_index, second_index, third_index, even?0:1, even?1:0, 2);
                        even = !even;
                    }
                    break;
                }
            }
#undef IND
        }
    };
}