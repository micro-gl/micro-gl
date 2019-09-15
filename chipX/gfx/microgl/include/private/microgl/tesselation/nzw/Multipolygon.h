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

/*
struct Rect
{
    Coord x0, y0, x1, y1;
    Rect(const Coord _x0 = 0, const Coord _y0 = 0,
         const Coord _x1 = 0, const Coord _y1 = 0)
            : x0(_x0), y0(_y0), x1(_x1), y1(_y1) {}
    Rect(const Vertex &v0, const Vertex &v1): x0(v0.x), y0(v0.y), x1(v1.x), y1(v1.y) {}
    Rect(const Rect &r): x0(r.x0), y0(r.y0), x1(r.x1), y1(r.y1) {}
    void normalize()
    {
        if (x0 > x1)
            swap(x0,x1);
        if (y0 > y1)
            swap(y0,y1);
    }

    // Precondition: rectangle is normalized
    bool isVertexInside(const Vertex &v) const {
        return v.x >= x0 && v.x <= x1 && v.y >= y0 && v.y <= y1;
    }

    // Precondition: rectangle is normalized
    void inflate(const Coord &dx, const Coord &dy) {
        x0 -= dx;
        x1 += dx;
        y0 -= dy;
        y1 += dy;
    }
};
 */

class LineSegment
{
    static const float NOISE; // used for determining limit of parallel lines
public:
    Vertex vertex0, vertex1;
    bool m_swappedVertices; // have vertices been swapped during sortVertices?
    Coord ymin, ymax; // y component of bounding box [don't use before call to sortVertices()]

    typedef enum
    {
        PARALLEL, // lines are parallel within tolerance level
        NO_INTERSECT, // lines segments don't intersect
        INTERSECT // line segments intersect
    } IntersectionType;

    LineSegment(){}

    // Sets bounding box of line segment to (vertex0.x, ymin, vertex1.x, ymax)
    void sortVertices();
    LineSegment(const Vertex &vtx0, const Vertex &vtx1);
    LineSegment(const LineSegment &l);

    IntersectionType calcIntersection(const LineSegment &l, Vertex &intersection,
                                      float &alpha, float &alpha1);

//    bool isVertexNear(const Vertex &p, const Coord &resolution);

    // order by left edge of bounding box
    bool operator< (const LineSegment &ls) const;
};
typedef list<Vertex>::iterator VtxIt;

// Intersection - to create the master list
// The most important object is an intersection object, which contains information about
class Intersection
{
public:
    // intersection point
    Vertex v;

    float param1,param2;
    // the pointers to the next intersection objects i.e., the value of
    // the index position in the intersection master list
    int index1, index2;
    // the starting vertices of the two intersecting polygon edges
    Vertex origin1, origin2;
    int winding;
    int direction;
    int selfIndex;
    LineSegment l1, l2;

    Intersection(){}
    Intersection (const Vertex &vtx, Vertex &org1, Vertex &org2,
                  float &p1,float &p2, LineSegment &li, LineSegment &lj);
    Intersection ( const Vertex &vtx, LineSegment &li, LineSegment &lj);
    Intersection( const Intersection &i): v(i.v), index1(i.index1), index2(i.index2),
                                          param1(i.param1),param2(i.param2), origin1(i.origin1), origin2(i.origin2),
                                          l1(i.l1), l2( i.l2 ), winding (i.winding), direction (i.direction),
                                          selfIndex(i.selfIndex) {}
    bool operator< (const Intersection &i) const;
    bool operator<= (const Intersection &i) const;
    void operator= ( const Intersection &i);
};

typedef vector<Intersection> ::iterator InterIt;

// nVertex - temporary class to create the objects in the
// polygon edge array
// every pseudo-vertex in the input polygon.
// The array of polygon
// edges is implemented as a vector (nVertex objects holding polygon vertices), each having
// a pointer to a linked list of intersections (nVertex objects of intersections), as shown in
// Figure 12 . It is implemented with the nVertex objects that hold necessary information to
// create the master list.

class nVertex
{
public:
    // polygon/intersection pseudo-vertex
    Vertex v;
    // the parametric value of intersection
    float param;
    // the index of the position of the pseudovertex in the intersection master list
    // this is the pointer into the linked-list of intersections of this vertex-edge
    int index;
    // the intersecting line segment
    LineSegment l;

    nVertex();
    nVertex ( const Vertex &vtx, float &p, int & i, LineSegment &l2 );
    nVertex ( const Vertex &vtx, int & i, LineSegment &l2 );
    nVertex ( const Vertex &vtx, LineSegment &l2 );
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
    bool operator== (const Pseudovertex &p) const;
};

typedef vector<Pseudovertex>::iterator PseudoIt;

// Poly: polygon; a cyclic collection of vertices
class Poly
{
public:
    list<Vertex> vtxList;
    Poly(const Poly &p);
    Poly();
    Poly(const Vertex &vtx, Coord resolution);
    void nextCirc(list<Vertex>::iterator &it);

    void prevCirc(list<Vertex>::iterator &it);

    int size() const;
};

typedef list<Poly>::iterator PolyIt;

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
    list<Poly> m_polyList{};

//    struct MPPos
//    {
//        list<Poly>::iterator m_polyIt;
//        list<Vertex>::iterator m_vtxIt;
//        MPPos(): m_vtxIt(nullptr), m_polyIt(NULL) {}
//        MPPos() = default;
//        MPPos(PolyIt pi, VtxIt vi): m_polyIt(pi), m_vtxIt(vi) {}
//        bool operator== (const MPPos pos) const;
//        bool operator!= (const MPPos pos) const;
//        void operator= (const MPPos pos);
//        bool isValid() const
//        {
//            return m_polyIt != NULL;
//        }
//    };
    MultiPoly(const MultiPoly &mp);
    MultiPoly()= default;

    void add(const Poly &p) {
        m_polyList.push_back(p);
    }
    vector<Vertex> findIntersections(vector<Intersection> &tempList);
    vector<Vertex> findMonotone(MultiPoly &resMPoly, vector<int> &windingVector);
    static void fillAddress(IntersectionList &ivList, vector <Intersection> &interVector);
    static void fillIndices(IntersectionList &ivList, vector <Intersection> &interVector);
    static LineSegment::IntersectionType findIntersection( LineSegment &l1, LineSegment &l2,
                                                    Vertex &intersection, float &alpha1, float &alpha2);
    static void polygonPartition(MultiPoly &resMPoly, vector<Intersection> &tempList,
                          vector<int> &windingVector);

    // cross-product, pq x qr
    static float xProd(const Vertex &p, Vertex &q, Vertex &r);
};

// impl

