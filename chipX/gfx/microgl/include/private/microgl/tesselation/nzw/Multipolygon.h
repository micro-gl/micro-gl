// Multipolyon.h
// Written by T. Hain. Extended by L. Subramaniam, Spring, 2003
#pragma once

#include <list>
#include <vector>
#include <queue>
#include <ctime>
#include <algorithm>
#include <stack>
#include <cmath>
using namespace std;
typedef float Coord;
class Vertex//: public CObject
{
public:
    Coord x, y;
    Vertex(const Coord _x = 0, const Coord _y = 0);
    Vertex(const Vertex &v);
    void operator= (const Vertex &v);
    Vertex operator+ (const Vertex &v) const;

    bool operator== ( const Vertex &v) const;
    bool operator!= (const Vertex &v) const;
    bool operator< (const Vertex &v) const;
    bool operator<= (const Vertex &v) const;
    bool operator> (const Vertex &v) const;

    Vertex& snap(Coord resolution);
};

class LineSegment
{
    static const float NOISE; // used for determining limit of parallel lines
public:
    Vertex *vertex0= nullptr, *vertex1= nullptr;
    // have vertices been swapped during sortVertices?
    bool m_swappedVertices=false;

    LineSegment()= default;

    typedef enum
    {
        PARALLEL, // lines are parallel within tolerance level
        NO_INTERSECT, // lines segments don't intersect
        INTERSECT // line segments intersect
    } IntersectionType;

    enum class bbox_axis {
        overlaps,
        start_of,
        end_of,
    };

    Vertex * start() {
        return !m_swappedVertices ? vertex0 : vertex1;
    }

    Vertex * end() {
        return !m_swappedVertices ? vertex1 : vertex0;
    }

    Vertex * start() const {
        return !m_swappedVertices ? vertex0 : vertex1;
    }

    Vertex * end() const {
        return !m_swappedVertices ? vertex1 : vertex0;
    }

    // Sets bounding box of line segment to (vertex0.x, ymin, vertex1.x, ymax)
    void sortVertices();
    LineSegment(Vertex *vtx0, Vertex *vtx1);

    IntersectionType calcIntersection(const LineSegment &l, Vertex &intersection,
                                      float &alpha, float &alpha1);

    bool is_bbox_overlapping_with(const LineSegment &a) {
        return classify_aligned_segment_relative_to(a, true)==bbox_axis::overlaps &&
                classify_aligned_segment_relative_to(a, false)==bbox_axis::overlaps;
    }

    bbox_axis classify_horizontal(const LineSegment &a) {
        return classify_aligned_segment_relative_to(a, true);
    }

    bbox_axis classify_vertical(const LineSegment &a) {
        return classify_aligned_segment_relative_to(a, false);
    }

    bbox_axis classify_aligned_segment_relative_to(const LineSegment &a, bool compare_x) {
        auto min_a = compare_x ? a.start()->x : a.start()->y;
        auto max_a = compare_x ? a.end()->x : a.end()->y;
        auto min_me = compare_x ? this->start()->x : this->start()->y;
        auto max_me = compare_x ? this->end()->x : this->end()->y;

        if(min_a > max_a) {
            auto temp = min_a;
            min_a = max_a;
            max_a = temp;
        }

        if(min_me > max_me) {
            auto temp = min_me;
            min_me = max_me;
            max_me = temp;
        }

        if(max_me <= min_a)
            return bbox_axis::start_of;
        else if(min_me >= max_a)
            return bbox_axis::end_of;
        else
            return bbox_axis::overlaps;
    }

    bool has_mutual_endpoint(const LineSegment &a) {
        if(*a.start()==*this->start() || *a.start()==*this->end())
            return true;
        return *a.end() == *this->start() || *a.end() == *this->end();
    }

    // order by left edge of bounding box
    bool operator< (const LineSegment &ls) const;
};
typedef vector<Vertex>::iterator VtxIt;

// Intersection - to create the master list
// The most important object is an intersection object, which contains information about
class Intersection
{
public:
    // intersection point
    Vertex *v{};

    float param1{},param2{};
    // the pointers to the next intersection objects i.e., the value of
    // the index position in the intersection master list
    int index1{}, index2{};
    // the starting vertices of the two intersecting polygon edges
    int winding{};
    int direction{};
    int selfIndex{};
    LineSegment l1, l2;

    Vertex * origin1() {
        return l1.start();
    }

    Vertex * origin2() {
        return l2.start();
    }

    Intersection()= default;
    Intersection (Vertex *vtx, /*Vertex *org1, Vertex *org2 */float p1,float p2, const LineSegment &li, const LineSegment &lj);
    Intersection (  Vertex *vtx, const LineSegment &li, const LineSegment &lj);
    bool operator< (const Intersection &i) const;
    bool operator<= (const Intersection &i) const;
    void operator= ( const Intersection &i);
};

typedef vector<Intersection> ::iterator InterIt;

class nVertex
{
public:
    // polygon/intersection pseudo-vertex
    Vertex *v;
    // the parametric value of intersection, this can be omitted with some more work
    // because we have the index in the intersection masterlist and therefore can
    // take this info from there
    float param;
    // the index of the position of the pseudovertex in the intersection master list
    // this is the pointer into the intersections master-list of this vertex-edge
    int index;
    // the intersecting line segment
//    LineSegment l;

    nVertex();
    nVertex ( Vertex *vtx, float p, int i);
    void operator= ( const nVertex &i);
    bool operator< (const nVertex &n) const;
    bool operator<= (const nVertex &i) const;
};

// Pseudovertex - the polygon edge array
//
class Pseudovertex
{
public:
    vector<nVertex> ilist;
    Pseudovertex();
    Pseudovertex ( const nVertex &n );
    Pseudovertex ( const Pseudovertex &p );
    bool operator< (const Pseudovertex &s) const;
//    bool operator== (const Pseudovertex &p) const;
};

typedef vector<Pseudovertex>::iterator PseudoIt;

// Poly: polygon; a cyclic collection of vertices
class Poly
{
public:
    vector<Vertex> vtxList;
    Poly(const Poly &p);
    Poly();
    Poly(const Vertex &vtx, Coord resolution);
    void nextCirc(vector<Vertex>::iterator &it);

    void prevCirc(vector<Vertex>::iterator &it);

    int size() const;
};

typedef vector<Poly>::iterator PolyIt;

// IntersectionList - list of polygon edges
class IntersectionList
{
public:
    vector<Pseudovertex> p_list;
    IntersectionList ();
    IntersectionList( const vector<Pseudovertex> &p);
};

// MultiPolygon: collection of polygons
class MultiPoly
{
public:
    vector<Poly> m_polyList{};
    vector<Vertex *> v_interesections{};

    MultiPoly(const MultiPoly &mp);
    MultiPoly()= default;

    void add(const Poly &p) {
        m_polyList.push_back(p);
    }
    vector<Vertex *> findIntersections(vector<Intersection> &tempList);
    void findMonotone(MultiPoly &resMPoly, vector<int> &windingVector,
                                vector<int> &directions);
    static void fillAddress(IntersectionList &ivList, vector <Intersection> &interVector);
    static void fillIndices(IntersectionList &ivList, vector <Intersection> &interVector);
    static void polygonPartition(MultiPoly &resMPoly, const vector<Intersection> &tempList,
                                 vector<int> &windingVector,
                                 vector<int> &directions);

    // cross-product, pq x qr
    static float xProd(const Vertex &p, Vertex &q, Vertex &r);
};

// impl

