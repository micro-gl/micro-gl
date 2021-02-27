#include <microgl/tesselation/Segment.h>

Segment::Segment(const vec2_32i &p0_, const vec2_32i &p1_) {
    p0 = p0_;
    p1 = p1_;
}

bool Segment::operator==(const Segment &rhs) const {
    return p0==rhs.p0 && p1==rhs.p1;
}

Rational Segment::compute_slope() const {
    long dy = isHorizontalLine() ? 0 : p1.y - p0.y;
    return Rational(p1.x - p0.x)/dy;
}

bool Segment::isHorizontalLine() const {
    long dx = p1.x - p0.x;
    long dy = p1.y - p0.y;
    Rational slope = {dx, dy};
//        bool slope_is_flatty =  dy!=0 && dx!=0 && slope.absolute() >= (1<<PR);
    bool isStraight = dy==0;// || slope_is_flatty;

    return isStraight;
}

Rational Segment::compute_x_intersection_with(const vec2_32i &p) const {
    // we presume we already have intersection with p.y
    Rational slope = compute_slope();//{dx, dy};
    auto dy = slope.denominator;
    Rational x2{10,10};

    // infinite slope
    if (isHorizontalLine())
        x2 = Rational::clamp(p.x, min(p0.x, p1.x), max(p0.x, p1.x));
    else {
        x2 = Rational{p0.x} + Rational(p.y - p0.y)*slope;
    }

    return {x2};
}

bool Segment::isStart(const vec2_32i &p) {
    return p0==p;
}

bool Segment::isEnd(const vec2_32i &p) {
    return p1==p;
}

bool Segment::contains(const vec2_32i &p) {
    bool lies_on_ray;

    if(isHorizontalLine()) {
        auto minY = min(p0.y, p1.y);
        auto maxY = max(p0.y, p1.y);
        lies_on_ray = p.y>=minY && p.y<=maxY;//EE;
    }
    else {

        Rational x_1 = compute_x_intersection_with({p.x, p.y});
        Rational x_2 = compute_x_intersection_with({p.x, p.y + 1});

        x_1.makeDenominatorPositive();
        x_2.makeDenominatorPositive();

        const auto & min_1 = Rational::minimum(x_1, x_2).toFixed();
        const auto & max_1 = Rational::maximum(x_1, x_2).toFixed();
        const auto & min_2 = Rational{p.x, 1}.toFixed();
        const auto & max_2 = Rational{p.x, 1}.toFixed();

        bool lines_intersect_on_p = (min_2 - max_1)<=1 && (min_1-max_2)<=1;
        lies_on_ray = lines_intersect_on_p;
    }

    // check edge cases beyond the segment
    if(lies_on_ray) {
        auto minX = min(p0.x, p1.x);
        auto minY = min(p0.y, p1.y);
        auto maxX = max(p0.x, p1.x);
        auto maxY = max(p0.y, p1.y);

        bool outside_x = minX > p.x || maxX < p.x;
        bool outside_y = minY > p.y || maxY < p.y;

        if(outside_x || outside_y)
            return false;
    }

    return lies_on_ray;
}

bool Segment::intersect(const Segment &rhs, vec2<Rational> &intersection) const {
    vec2_rat s1 = p1 - p0;
    vec2_rat s2 = rhs.p1 - rhs.p0;
    vec2_rat dc = p0 - rhs.p0;

    Rational d = -s2.x * s1.y + s1.x * s2.y;
    Rational s = (-s1.y * dc.x + s1.x * dc.y) / d;
    Rational t = ( s2.x * dc.y - s2.y * dc.x) / d;

    if (s.isRegular() && t.isRegular() && s >= 0 && s <= 1 && t >= 0 && t <= 1) {

        // try this later
        intersection = vec2_rat{p0} + s1 * t;

        return true;
    }

    return false;
}

bool Segment::intersect(const Segment &rhs, vec2_f & intersection) const {
    vec2_f s1 = p1 - p0; //ab
    vec2_f s2 = rhs.p1 - rhs.p0; // cd
    vec2_f dc = p0 - rhs.p0; // ca

    float d = -s2.x * s1.y + s1.x * s2.y;
    float s = (-s1.y * dc.x + s1.x * dc.y) / d; // (-ab.y*ca.x + ab.x*ca.y)
    float t = ( s2.x * dc.y - s2.y * dc.x) / d;

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        intersection = vec2_f{p0} + s1 * t;

        return true;
    }

    return false; // No collision
}
