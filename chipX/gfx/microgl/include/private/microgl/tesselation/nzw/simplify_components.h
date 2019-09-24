// Multipolyon.h
// Written by T. Hain. Extended by L. Subramaniam, Spring, 2003
#pragma once

#include <list>
#include <vector>
#include <ctime>
#include <algorithm>
#include <stack>
#include <cmath>
#include <microgl/vec2.h>
using namespace std;

struct Vertex
{
public:
    float x, y;
    Vertex(float _x = 0, float _y = 0) : x(_x), y(_y) {}
};

class segment
{
    static const float NOISE; // used for determining limit of parallel lines
public:
    Vertex *vertex0= nullptr, *vertex1= nullptr;
    // have vertices been swapped during sortVertices?
    bool m_swappedVertices=false;

    segment()= default;

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
    segment(Vertex *vtx0, Vertex *vtx1);

    IntersectionType calcIntersection(const segment &l, Vertex &intersection,
                                      float &alpha, float &alpha1);

    bool is_bbox_overlapping_with(const segment &a) {
        return classify_aligned_segment_relative_to(a, true)==bbox_axis::overlaps &&
                classify_aligned_segment_relative_to(a, false)==bbox_axis::overlaps;
    }

    bbox_axis classify_horizontal(const segment &a) {
        return classify_aligned_segment_relative_to(a, true);
    }

    bbox_axis classify_vertical(const segment &a) {
        return classify_aligned_segment_relative_to(a, false);
    }

    bbox_axis classify_aligned_segment_relative_to(const segment &a, bool compare_x) {
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

    // this might be done with just pointers compare instead ?
    bool has_mutual_endpoint(const segment &a) {
        bool a_start_start = a.start()->x==this->start()->x && a.start()->y==this->start()->y;
        bool a_start_end = a.start()->x==this->end()->x && a.start()->y==this->end()->y;

        if(a_start_start || a_start_end)
            return true;
        else {
            bool a_end_start = a.end()->x==this->start()->x && a.end()->y==this->start()->y;
            bool a_end_end = a.end()->x==this->end()->x && a.end()->y==this->end()->y;

            return a_end_start || a_end_end;
        }

//        if(*a.start()==*this->start() || *a.start()==*this->end())
//            return true;
//        return *a.end() == *this->start() || *a.end() == *this->end();
    }

    // order by left edge of bounding box
    bool operator< (const segment &ls) const;
};

// intersection - to create the master list
// The most important object is an intersection object,
// which contains information about
struct intersection
{
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
    segment l1, l2;

    Vertex * origin1() {
        return l1.start();
    }

    Vertex * origin2() {
        return l2.start();
    }

    intersection()= default;
    intersection (Vertex *vtx, float p1, float p2, const segment &li, const segment &lj);
    bool operator< (const intersection &i) const;
};

struct edge_vertex
{
    // polygon/intersection pseudo-vertex
    Vertex *v;
    // the parametric value of intersection, this can be omitted with some more work
    // because we have the index in the intersection masterlist and therefore can
    // take this info from there
    float param;
    // the index of the position of the pseudovertex in the intersection master list
    // this is the pointer into the intersections master-list of this vertex-edge
    int index;

    edge_vertex (Vertex *vtx, float p, int i);
    bool operator< (const edge_vertex &n) const;
};

// edge - the polygon edge array
//
struct edge
{
    vector<edge_vertex> vertices;
    edge() = default;
    bool operator< (const edge &s) const;
};

// Poly: polygon; a cyclic collection of vertices
class Poly
{
public:
    vector<Vertex> vtxList;
    Poly(const Poly &p);
    Poly() = default;

    int size() const;
};

// edge_list - list of polygon edges
using edge_list = vector<edge>;
using master_intersection_list = vector<intersection>;

// MultiPolygon: collection of polygons
class MultiPoly
{
public:
    vector<Poly> m_polyList{};
    vector<Vertex *> v_interesections{};

    MultiPoly()= default;

    void add(const Poly &p) {
        m_polyList.push_back(p);
    }
    vector<Vertex *> findIntersections(vector<intersection> &master_list);
    void findMonotone(MultiPoly &resMPoly, vector<int> &windingVector,
                                vector<int> &directions);
    static void fillAddress(edge_list &ivList, master_intersection_list &master_list);
    static void polygonPartition(MultiPoly &resMPoly,
                                 vector<intersection> &tempList,
                                 vector<int> &windingVector,
                                 vector<int> &directions);

    // cross-product, pq x qr
    static float xProd(const Vertex &p, Vertex &q, Vertex &r);
};

#include <microgl/chunker.h>
class simplify_components {
    using index = unsigned int;

    simplify_components(const chunker<microgl::vec2_f> & pieces,
                        chunker<microgl::vec2_f> & pieces_result,
                        vector<int> &winding) {

    }

};