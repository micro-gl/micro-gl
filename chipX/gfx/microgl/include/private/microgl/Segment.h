#pragma once

#include <microgl/Types.h>
#include <microgl/Rational.h>

#define max(a,b) (a)>(b) ? (a) : (b)
#define min(a,b) (a)<(b) ? (a) : (b)

typedef vec2<Rational> vec2_rat;

struct Segment {
    vec2_32i p0, p1;

    Segment() = default;
    Segment(const Segment & val) = default;
    Segment(const vec2_32i & p0_, const vec2_32i & p1_);

    bool operator==(const Segment & rhs) const;

    Rational compute_slope() const;
    bool isHorizontalLine() const;
    // special intersection with the ray encapsulating the segment
    Rational compute_x_intersection_with(const vec2_32i & p) const;
    bool contains(const vec2_32i & p);
    bool isStart(const vec2_32i & p);
    bool isEnd(const vec2_32i & p);

    bool intersect(const Segment& rhs,
            vec2<Rational> & intersection) const;

    bool intersect(const Segment& rhs,
                   vec2_f &intersection) const;

};
