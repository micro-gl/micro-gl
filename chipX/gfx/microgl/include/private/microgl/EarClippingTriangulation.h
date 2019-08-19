#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <algorithm>
#include <microgl/Types.h>
#include <microgl/AVLTree.h>
#include <microgl/Rational.h>
#include <microgl/Segment.h>
#include <vector>
#include <iostream>

namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))

    class EarClippingTriangulation {
    public:
        using index = unsigned int;

        explicit EarClippingTriangulation(bool DEBUG = false) : _DEBUG{DEBUG} {};

        // t
        // positive if CCW
        long long orientation_value(index i,
                                    index j,
                                    index k)
        {
            return pts[i].x * (pts[j].y - pts[k].y) +
                   pts[j].x * (pts[k].y - pts[i].y) +
                   pts[k].x * (pts[i].y - pts[j].y);
        }

        int neighborhood_orientation_sign(index v)
        {
            index l = predecessor(v), r = successor(v);

            // pay attention that this can return 0, although in the algorithm
            // it does not return 0 never here.
            return sign_orientation_value(l, v, r) > 0 ? 1 : -1;
        }

        // tv
        char sign_orientation_value(index i, index j, index k) {
            auto v = orientation_value(i, j, k);

            // we clip, to avoid overflows down the road
            if(v > 0)
                return 1;
            else if(v < 0)
                return -1;
            else
                return 0;
        }

        index predecessor(index v) {
            if(int(v)-1 == -1)
                return size;

            return v - 1;
        }

        index successor(index v) {
            if(int(v)+1 == size)
                return 0;

            return v + 1;
        }

        // main

        index maximal_y_element() {
            index maximal_index = 0;
            int maximal_y = pts[maximal_index].y;

            for (unsigned int ix = 0; ix < size; ++ix) {
                if(pts[ix].y > maximal_y) {
                    maximal_y = pts[ix].y;
                    maximal_index = ix;
                }
            }

            return maximal_index;
        }

        bool isConvex(index v)
        {
            return neighborhood_orientation_sign(v) *
                    neighborhood_orientation_sign(maximal_y_element()) > 0;
        }

        bool isEmpty(index v)
        {
            int tsv;

            index l = predecessor(v);
            index r = successor(v);

            tsv = sign_orientation_value(v, l, r);

            for(index i = 0; i < size ; i++)
            {
                if(i==v || i==l || i==r)
                    continue;

                if(tsv * sign_orientation_value(v, l, i)>=0 &&
                   tsv * sign_orientation_value(l, r, i)>=0 &&
                   tsv * sign_orientation_value(r, v, i)>=0
                   )
                    return false;

            }

            return true;
        }

        void prune(index v)
        {
            size--;

            for(index i=v; i < size; i++) {
                pts[i].x = pts[i+1].x;
                pts[i].y = pts[i+1].y;
            }

        }


        std::vector<vec2_32i> & compute(vec2_32i * $pts,
                                        int $size,
                                        uint8_t precision) {
            pts = $pts;
            size = $size;

            return compute_internal();
        }

        std::vector<vec2_32i> & compute_internal() {

            for (index ix = 0; ix < size - 3; ++ix) {

                for (index jx = 0; jx < size; jx++) {

                    if (isConvex(jx) && isEmpty(jx)) {

                        prune(jx);

                        break;
                    }

                }

            }

            destroy();

            return I;
        }


        void destroy() {
            pts = nullptr;
            size = 0;
        }

        std::vector<vec2_32i> & getIntersections() {
            return I;
        }

    private:
        vec2_32i * pts = nullptr;
        unsigned int size = 0;

        bool _DEBUG = false;
        std::vector<vec2_32i> I;

        uint8_t PR;
    };


}

#pragma clang diagnostic pop