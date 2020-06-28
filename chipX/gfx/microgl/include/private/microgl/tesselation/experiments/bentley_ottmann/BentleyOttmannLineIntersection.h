#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma once

#include <microgl/vec2.h>
#include <microgl/tesselation/AVLTree.h>
#include <microgl/tesselation/Rational.h>
#include <microgl/tesselation/Segment.h>
#include <vector>
#include <iostream>

namespace experiments {

#define abs(a) ((a)<0 ? -(a) : (a))

    enum class event_type_t {
        START, END, Intersection
    };

    typedef vec2<Rational> vec2_rat;

    struct segment_order
    {

        long cmp(const Segment& lhs, const Segment& rhs);

        bool isPreceding(const Segment& lhs, const Segment& rhs)
        {
            return cmp(lhs, rhs) < 0;
        }

        void updateComparePoint(const vec2_32i & val) {
            p = val;
        }

        bool isCompareOnTheLine() {
            return _compare_on_the_line;
        }

        void compareOnTheLine(bool val) {
            _compare_on_the_line = val;
        }

        const vec2_32i & getComparePoint() {
            return p;
        }
    private:
        int EE = 0;
        bool _compare_on_the_line = false;
        vec2_32i p;
    };

    struct event_point_t {
        Segment segment{};
        event_type_t type = event_type_t::START;

        const vec2_32i & getPoint() const {
            switch (type) {
                case event_type_t::START:
                    return segment.p0;
                case event_type_t::END:
                    return segment.p1;
                default:
                    return segment.p0;
            }

        }

    };

    struct event_order
    {
        bool isPreceding(const event_point_t& lhs, const event_point_t& rhs)
        {
            return (lhs.getPoint().y < rhs.getPoint().y) ||
                   ((lhs.getPoint().y == rhs.getPoint().y) && (lhs.getPoint().x < rhs.getPoint().x));
        }

    };

    class BentleyOttmann {
    public:
        int PR = 1;

        explicit BentleyOttmann(bool DEBUG = false);;

        event_point_t create_event(const vec2_f & p0,
                                   const vec2_f & p1,
                                   event_type_t type,
                                   uint8_t precision = 4);

        event_point_t create_event(const vec2_32i & p0,
                                   const vec2_32i & p1,
                                   event_type_t type,
                                   uint8_t precision = 4);

        event_point_t create_event(const Segment & segment,
                                   event_type_t type,
                                   uint8_t precision = 4);

        event_point_t create_event(const vec2_rat & p0,
                                   const vec2_rat & p1,
                                   event_type_t type,
                                   uint8_t precision = 4);

        std::vector<vec2_32i> & compute(vec2_f * pts,
                                        int size,
                                        uint8_t precision);

        std::vector<vec2_32i> & compute(vec2_32i * pts,
                                        int size,
                                        uint8_t precision);

        std::vector<vec2_32i> & compute(Segment * seg,
                                        int size,
                                        uint8_t precision);

        std::vector<vec2_32i> & compute_internal();

        void handleEventPoint(event_point_t & event);

        void find_new_event(const Segment & l,
                            const Segment & r,
                            const vec2_32i & p);

        void destroy();

        bool hasTheAlgorithmFailed();

        std::vector<vec2_32i> & getIntersections();

    private:
        typedef ds::AVLTree<event_point_t, event_order> PriorityQueue;
        typedef ds::AVLTree<Segment, segment_order> Status;
        bool _DEBUG = false;
        bool _has_failed_for_sure = false;
        PriorityQueue Queue;
        Status S;
        std::vector<vec2_32i> I;
    };

}
#pragma clang diagnostic pop