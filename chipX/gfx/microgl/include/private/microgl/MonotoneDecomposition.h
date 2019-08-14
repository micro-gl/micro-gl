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

    namespace monotone {

        enum class event_type_t {
            START, END, Intersection
        };

        enum class direction_t {
            CW, CCW
        };

        template<typename T>
        T * ptr(T & obj) { return &obj; }

        template<typename T>
        T * ptr(T * obj) { return obj; }

        template <typename T>
        struct edge_t {
            vec2<T> * p0 = nullptr, * p1 = nullptr;
            edge_t<T> * prev = nullptr;
            edge_t<T> * next = nullptr;
        };

        template <typename T>
        struct event_point_t {
            vec2<T> * p;
            edge_t<T> * edge = nullptr;
        };

        template <typename T>
        struct event_order_t
        {
            bool isPreceding(const event_point_t<T>& lhs,
                             const event_point_t<T>& rhs)
            {
                return (lhs.y < rhs.y) ||
                       ((lhs.y == rhs.y) && (lhs.x < rhs.x));
            }

        };

        template <typename T>
        struct status_point_t {
            edge_t<T> * edge = nullptr;
            vec2<T> * helper = nullptr;
        };

        template <typename T>
        struct edge_list_t {
            edge_t<T> * root = nullptr;
        };

        typedef vec2<Rational> vec2_rat;

        template <typename T>
        struct status_order_t
        {
            int EE = 1;
            long cmp(const Segment& lhs, const Segment& rhs) {

                Rational lhs_x = lhs.compute_x_intersection_with(p);
                Rational rhs_x = rhs.compute_x_intersection_with(p);
                Rational lhs_x_at_1 = lhs.compute_x_intersection_with({p.x, p.y + 1});
                Rational rhs_x_at_1 = rhs.compute_x_intersection_with({p.x, p.y + 1});

                lhs_x.makeDenominatorPositive();
                rhs_x.makeDenominatorPositive();
                lhs_x_at_1.makeDenominatorPositive();
                rhs_x_at_1.makeDenominatorPositive();

                // let's test if rhs passes through the unit cube which has
                // top-left at (lhs_x, p.y) ?

                const auto & min_1 = Rational::minimum(lhs_x, lhs_x_at_1).toFixed();
                const auto & max_1 = Rational::maximum(lhs_x, lhs_x_at_1).toFixed();
                const auto & min_2 = Rational::minimum(rhs_x, rhs_x_at_1).toFixed();
                const auto & max_2 = Rational::maximum(rhs_x, rhs_x_at_1).toFixed();

                bool lines_intersect_on_p = (min_2 - max_1)<=EE && (min_1 - max_2)<=EE;

                // may use it in the future
                bool x_ref_in_1 = (p.x - max_1)<=EE && (min_1 - p.x)<=EE;//(p.x>=min_1 && p.x<=max_1) || (p.x+1>=min_1 && p.x+1<=max_1);
                bool x_ref_in_2 = (min_2 - p.x)<=EE && (p.x - max_2)<=EE;//(p.x>=min_2 && p.x<=max_2) || (p.x+1>=min_2 && p.x+1<=max_2);

                bool inside = lines_intersect_on_p;// && x_ref_in_1 && x_ref_in_2;

                if(!inside) {
                    Rational compare = (lhs_x - rhs_x);
                    compare.makeDenominatorPositive();
                    return compare.numerator;
                }

                // if they are equal at p, we need to break tie with slope
                Rational lhs_slope = lhs.compute_slope();
                Rational rhs_slope = rhs.compute_slope();

                // return

                lhs_slope.makeDenominatorPositive();
                rhs_slope.makeDenominatorPositive();

                long tie = 0;

                // I want to clarify that we never deal with negative infinity slopes
                if((lhs_slope.isPositiveInfinity() && rhs_slope.isPositiveInfinity()) ||
                   (lhs_slope.isNegativeInfinity() && rhs_slope.isNegativeInfinity())
                        )
                    return 0;

                if(lhs_slope.isPositiveInfinity())
                    return 1;
                else if(rhs_slope.isNegativeInfinity())
                    return 1;
                else if(rhs_slope.isPositiveInfinity())
                    return -1;
                else if(lhs_slope.isNegativeInfinity())
                    return -1;
                else {
                    tie = (lhs_slope - rhs_slope).numerator;
                }

                return isCompareOnTheLine() ? -tie : tie;
            }

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
            bool _compare_on_the_line = false;
            vec2<T> p;
        };

        template <typename T>
        class MonotoneDecomposition {
        public:

            using event_point = event_point_t<T>;
            using edge = edge_t<T>;
            using status_point = status_point_t<T>;
            using event_order = event_order_t<T>;
            using status_order = status_order_t<T>;
            using edge_list = edge_list_t<T>;
            using PriorityQueue = std::vector<vec2<T>>;
            using Status = ds::AVLTree<status_point, status_order>;
            using StatusNode = typename ds::AVLTree<Segment, status_order>::Node;

            explicit MonotoneDecomposition(bool DEBUG = false) : _DEBUG{DEBUG} {};

            event_point create_event(const vec2_f & p0,
                                     const vec2_f & p1,
                                     event_type_t type,
                                     uint8_t precision = 4) {
                // convert to fixed precision
                vec2_32i p0_i, p1_i;

                // applying precision here
                p0_i = p0<<precision;
                p1_i = p1<<precision;

                return create_event(p0_i, p1_i, type, 0);
            }

            event_point create_event(const vec2_32i & p0,
                                     const vec2_32i & p1,
                                     event_type_t type,
                                     uint8_t precision = 4) {
                event_point pt;

                pt.segment.p0 = p0<<precision;
                pt.segment.p1 = p1<<precision;
                pt.type = type;

                bool reverseY = pt.segment.p1.y<pt.segment.p0.y;

                if(reverseY) {
                    std::swap(pt.segment.p0,pt.segment.p1);
                    pt.type = pt.type==event_type_t::START ? event_type_t::END : event_type_t::START;
                }

                return pt;
            }

            event_point create_event(const Segment & segment,
                                     event_type_t type,
                                     uint8_t precision = 4) {
                return create_event(segment.p0, segment.p1,
                                    type, precision);
            }

            /*
            event_point create_event(const vec2_rat & p0,
                                     const vec2_rat & p1,
                                     event_type_t type,
                                     uint8_t precision = 4) {
                event_point pt;

                pt.segment.p0 = {p0.x.toFixed(precision), p0.y.toFixed(precision)};
                pt.segment.p1 = {p1.x.toFixed(precision), p1.y.toFixed(precision)};
                pt.type = type;

                bool reverse = pt.segment.p1.y<pt.segment.p0.y;

                if(reverse) {
                    std::swap(pt.segment.p0,pt.segment.p1);
                    pt.type = type==event_type_t::START ? event_type_t::END : event_type_t::START;
                }

                return pt;
            }
            */

            /*
            std::vector<vec2_32i> & compute(vec2_f * pts,
                                            int size,
                                            uint8_t precision) {

                for (int ix = 0; ix < size; ++ix) {
                    event_point e1 = create_event(pts[ix], pts[ix+1],
                                                  event_type_t::START, precision);
                    event_point e2 = create_event(pts[ix], pts[ix+1],
                                                  event_type_t::END, precision);
                    Queue.insert(e1);
                    Queue.insert(e2);
                }

                return compute_internal();
            }
             */

            std::vector<vec2_32i> & compute(vec2_32i * pts,
                                            int size,
                                            uint8_t precision,
                                            const direction_t &direction = direction_t::CW) {

                for (int ix = 0; ix < size; ++ix) {
                    event_point e;

                    e.p = &pts[ix];

                    event_point e1 = create_event(pts[ix], pts[ix+1],
                                                  event_type_t::START, precision);
                    event_point e2 = create_event(pts[ix], pts[ix+1],
                                                  event_type_t::END, precision);
                    Queue.insert(e1);
                    Queue.insert(e2);
                }

                return compute_internal();
            }

            std::vector<vec2_32i> & compute_internal() {


                StatusNode *ne = S.findMin();

                Rational zero{0, 0};

                S.getComparator().updateComparePoint({0,-10});

                while (!Queue.isEmpty()) {
                    event_point event = Queue.removeMinKey();

                    if(_DEBUG) {
                        std::string info = std::to_string(event.getPoint().x) +
                                           "x" + std::to_string(event.getPoint().y);
                        std::cout<<"event p " << info << std::endl;
                    }

                    handleEventPoint(event);
                }

                if(!S.isEmpty())
                    _has_failed_for_sure = true;

                if(_has_failed_for_sure && _DEBUG) {
                    std::cout << "Algorithm probaly has Failed" << std::endl;
                }

                destroy();

                return I;
            }


            void destroy() {
                Queue.clear();
                S.clear();
            }

            bool hasTheAlgorithmFailed() {
                return _has_failed_for_sure;
            }

            std::vector<vec2_32i> & getIntersections() {
                return I;
            }

        private:

            bool _DEBUG = false;
            bool _has_failed_for_sure = false;
            PriorityQueue Queue;
            Status S;
            std::vector<vec2_32i> I;

            uint8_t PR;
        };

    }

}
#pragma clang diagnostic pop