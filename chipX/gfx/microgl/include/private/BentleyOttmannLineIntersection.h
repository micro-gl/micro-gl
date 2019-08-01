#pragma once

#include <Types.h>
#include "RedBlackTree.h"

namespace tessellation {

//    http://www.normalesup.org/~cagne/internship_l3/mncubes_red/doc/html/bentley__ottmann_8cpp_source.html

    enum class event_type_t {
        START, END
    };

    struct segment_t {
        vec2_32i p0, p1;
    };

    struct event_point_t {
        // experimenting

        segment_t segment;
        event_type_t type = event_type_t::START;

        const vec2_32i & getPoint() const {
            switch (type) {
                case event_type_t::START:
                    return segment.p0;
                case event_type_t::END:
                    return segment.p1;
            }

        }

//        vec2_32i p0;//, p1;
//        std::vector<segment_t> segments;
    };

    struct event_order
    {
        bool isPreceding(const event_point_t& lhs, const event_point_t& rhs)
        {
            return (lhs.getPoint().y < rhs.getPoint().y) ||
                    ((lhs.getPoint().y == rhs.getPoint().y) && (lhs.getPoint().x < rhs.getPoint().x));
        }

        bool isEqual(const event_point_t& lhs, const event_point_t& rhs)
        {
            return (lhs.getPoint().x==rhs.getPoint().x) && (lhs.getPoint().y==rhs.getPoint().y);
        }
    };

    #define max(a,b) (a)>(b) ? (a) : (b)

    float solve_x_between_given_y(vec2_32i &p0, vec2_32i &p1, int y) {
        float t = float(y - p0.y)/float(p1.y - p0.y);
        float x = p0.x + t*float(p1.x - p0.x);

        return x;
    }

    struct segment_order
    {
        bool isPreceding(const segment_t& lhs, const segment_t& rhs)
        {
            auto maxY = max(lhs.p0.y, rhs.p0.y);

            // this is an optimization to avoid divisions
            if(maxY==lhs.p0.y) {
                auto dx = rhs.p1.x - rhs.p0.x;
                auto dy = rhs.p1.y - rhs.p0.y;
                return (lhs.p0.x*dy) < rhs.p0.x*dy + (maxY - rhs.p0.y)*dx;
            } else {
                auto dx = lhs.p1.x - lhs.p0.x;
                auto dy = lhs.p1.y - lhs.p0.y;
                return (rhs.p0.x*dy) > lhs.p0.x*dy + (maxY - lhs.p0.y)*dx;
            }

        }

        bool isEqual(const segment_t& lhs, const segment_t& rhs)
        {
            return (
                    (lhs.p0.x==rhs.p0.x) && (lhs.p0.y==rhs.p0.y) &&
                    (lhs.p1.x==rhs.p1.x) && (lhs.p1.y==rhs.p1.y)
            );
        }
    };

    // Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
    // intersect the intersection point may be stored in the floats i_x and i_y.
    bool get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
                               float p2_x, float p2_y, float p3_x, float p3_y,
                               float *i_x=nullptr, float *i_y=nullptr)
    {
        float s1_x, s1_y, s2_x, s2_y;
        s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
        s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

        float s, t;
        s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
        t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

        if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
        {
            // Collision detected
            if (i_x != nullptr)
                *i_x = p0_x + (t * s1_x);
            if (i_y != nullptr)
                *i_y = p0_y + (t * s1_y);
            return true;
        }

        return false; // No collision
    }

    class BentleyOttmann {
    public:
        BentleyOttmann(vec2_32i * pts, int size) {
            for (int ix = 0; ix < size; ++ix) {
                Q.insert(create_event(pts[0], pts[1], event_type_t::START));
                Q.insert(create_event(pts[0], pts[1], event_type_t::END));
            }
        }

        event_point_t create_event(const vec2_32i & p0, const vec2_32i & p1, event_type_t type) {
            event_point_t pt;
            pt.segment.p0 = p0;
            pt.segment.p1 = p1;
            pt.type = type;

            return pt;
        }

        void line_intersection() {
            while (!Q.isEmpty()) {
                event_point_t event = Q.removeMinElement();

                /*
                // do this later to handle multiple
                while (Q.contains(event)) {
                    event_point_t event2 = Q.removeMinElement();
                    // insert this segment into event
                }
                */

                handleEventPoint(event);
            }

        }

        void handleEventPoint(event_point_t & event) {
            unsigned int score = 0;

            if(event.type==event_type_t::START)
                score += 1;


        }

    private:
        ds::RedBlackTree<event_point_t, event_order> Q;
        ds::RedBlackTree<segment_t, segment_order> S;
    };

}