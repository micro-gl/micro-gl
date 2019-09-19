#include <microgl/tesselation/nzw/Multipolygon.h>

Vertex::Vertex(Coord _x, Coord _y) : x(_x), y(_y) {}

Vertex::Vertex(const Vertex &v) : x(v.x), y(v.y) {}

void Vertex::operator=(const Vertex &v) { x = v.x; y = v.y; }

Vertex Vertex::operator+(const Vertex &v) const {
    return Vertex(x + v.x, y + v.y);
}

bool Vertex::operator==(const Vertex &v) const { return x == v.x && y == v.y; }

bool Vertex::operator!=(const Vertex &v) const { return x != v.x || y != v.y; }

bool Vertex::operator<(const Vertex &v) const { return x < v.x || (x == v.x && y < v.y); }

bool Vertex::operator<=(const Vertex &v) const { return x <= v.x; }

bool Vertex::operator>(const Vertex &v) const { return y < v.y || y == v.y ; }

Vertex &Vertex::snap(Coord resolution) // Snap coords to given resolution
{
    if (resolution)
    {
        x = resolution * (int)(x/resolution);
        y = resolution * (int)(y/resolution);
    }
    return *this;
}

Intersection::Intersection(const Vertex &vtx, Vertex &org1, Vertex &org2, float &p1, float &p2, LineSegment &li,
                           LineSegment &lj) {
    v = vtx;
    index1 = -1;
    index2 = -1;
    param1 = p1;
    param2 = p2;
    origin1 = org1;
    origin2 = org2;
    l1 = li;
    l2 = lj;
}

Intersection::Intersection(const Vertex &vtx, LineSegment &li, LineSegment &lj) {
    v = vtx;
    index1 = -1;
    index2 = -1;
    param1 = 0;
    param2 = 0;
    origin1 = v;
    l1 = li;
    l2 = lj;
    winding = 0;
    direction = 0;
    selfIndex = 0;
}

bool Intersection::operator<(const Intersection &i) const { return (v < i.v ); }

bool Intersection::operator<=(const Intersection &i) const { return (v <= i.v ); }

void Intersection::operator=(const Intersection &i) {
    v = i.v;
    index1 = i.index1;
    index2= i.index2;
    param1 = i.param1;
    param2 = i.param2;
    origin1 = i.origin1;
    origin2 = i.origin2;
    l1 = i.l1;
    l2 = i.l2;
    winding = i.winding;
    direction = i.direction;
    selfIndex = i.selfIndex;
}

nVertex::nVertex() {}

nVertex::nVertex(const Vertex &vtx, float &p, int &i, LineSegment &l2) {
    v = vtx;
    param = p;
    index = i;
    l = l2;
}

nVertex::nVertex(const Vertex &vtx, int &i, LineSegment &l2) {
    v = vtx;
    param = 0;
    index = i;
    l = l2;
}

nVertex::nVertex(const Vertex &vtx, LineSegment &l2) {
    v = vtx;
    l = l2;
}

void nVertex::operator=(const nVertex &i) {
    v = i.v;
    l = i.l;
    param= i.param;
    index = i.index;
}

bool nVertex::operator<(const nVertex &n) const { return (param < n.param ); }

bool nVertex::operator<=(const nVertex &i) const { return (param <= i.param ); }

Pseudovertex::Pseudovertex() {}

Pseudovertex::Pseudovertex(const nVertex &n) { ilist.push_back(n); }

Pseudovertex::Pseudovertex(const Pseudovertex &p) : ilist(p.ilist) {}

bool Pseudovertex::operator<(const Pseudovertex &s) const { return ( ilist.begin()->v < s.ilist.begin()->v ); }

bool Pseudovertex::operator==(const Pseudovertex &p) const { return ( ilist.begin()->v == p.ilist.begin()->v ); }

Poly::Poly(const Poly &p) : vtxList(p.vtxList){}

Poly::Poly() {}

Poly::Poly(const Vertex &vtx, Coord resolution) {
    vtxList.push_back(Vertex(vtx.x + 10.0f, vtx.y - 10.0f).snap(resolution));
    vtxList.push_back(Vertex(vtx.x - 10.0f, vtx.y - 10.0f).snap(resolution));
    vtxList.push_back(Vertex(vtx.x , vtx.y + 10.0f).snap(resolution));
}

void Poly::nextCirc(vector<Vertex>::iterator &it) // for circular linkage
{
    if (++it == vtxList.end())
        it = vtxList.begin();
}

void Poly::prevCirc(vector<Vertex>::iterator &it) // for circular linkage
{
    if (it == vtxList.begin())
        it = vtxList.end();
    --it;
}

int Poly::size() const {
    return vtxList.size();
}

IntersectionList::IntersectionList() {}

IntersectionList::IntersectionList(const vector<Pseudovertex> &p) : p_list(p) {}

const float LineSegment::NOISE = 1e-5f;
////////////////////// Line Intersection ///////////////////////////
//
// Calculates the point of intersection of two line segments.
//
// Precondition: both line segments have non-zero lengths.
// Postcondition: The enumerated type function return value tells whether the lines
// segments were parallel, non-intersecting, or intersecting.
// "intersectionPoint" is valid iff the returned type is not PARALLEL.
//
LineSegment::IntersectionType LineSegment::calcIntersection
        (const LineSegment &l, Vertex &intersection, float &alpha, float &alpha1)
{
    float dx21 = vertex1.x - vertex0.x, dy21 = vertex1.y - vertex0.y,
            dx43 = l.vertex1.x - l.vertex0.x, dy43 = l.vertex1.y - l.vertex0.y;
    float dem = dx21 * dy43 - dy21 * dx43;
// parallel lines
    if (fabs(dem) < NOISE)
        return PARALLEL;
    else {
        float dx12 = vertex0.x - l.vertex0.x,
                dy12 = vertex0.y - l.vertex0.y;
        float dx = l.vertex0.x - vertex0.x,
                dy = l.vertex0.y - vertex0.y;
        alpha = (dy12*dx43 - dx12*dy43)/dem;
        alpha1 = ( dy21*dx - dx21*dy)/dem;
// The intersecting point
        intersection.x = vertex0.x + dx21 * alpha;
        intersection.y = vertex0.y + dy21 * alpha;
// test for segment intersecting (alpha)
        if ((alpha < 0.0) || (alpha > 1.0))
            return NO_INTERSECT;
        else {
            float num = dy12*dx21 - dx12*dy21;
            if (dem > 0.0) {
                if (num < 0.0 || num > dem)
                    return NO_INTERSECT;
            }
            else {
                if (num > 0.0 || num < dem)
                    return NO_INTERSECT;
            }
        }
    }
    return INTERSECT;
}

void LineSegment::sortVertices() {
    m_swappedVertices = !(vertex0 < vertex1);
    if (m_swappedVertices)
        swap(vertex0, vertex1);
    if (vertex0.y < vertex1.y)
    {
        ymin = vertex0.y;
        ymax = vertex1.y;
    }
    else
    {
        ymin = vertex1.y;
        ymax = vertex0.y;
    }
}

LineSegment::LineSegment(const Vertex &vtx0, const Vertex &vtx1) : vertex0(vtx0), vertex1(vtx1) {}

LineSegment::LineSegment(const LineSegment &l) : vertex0(l.vertex0),vertex1(l.vertex1),ymin(l.ymin),
                                                 ymax(l.ymax), m_swappedVertices(l.m_swappedVertices) {}

bool LineSegment::operator<(const LineSegment &ls) const {
    return vertex0 < ls.vertex0;
}

/*

 // Snap all vertex coordinates to given resolution
void MultiPoly::gridify(Coord resolution)
{
    if (resolution != 0)
        for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
            for (VtxIt vi = pi->vtxList.begin(); vi != pi->vtxList.end(); ++vi)
                vi->snap(resolution);
}


 bool LineSegment::isVertexNear(const Vertex &vtx, const Coord &resolution)
{
// CRect activeRect(vertex0.m_point, vertex1.m_point);
// activeRect.NormalizeRect();
// activeRect.InflateRect(resolution, resolution); // for vertical or horizontal edges
// if (!activeRect.PtInRect(*this))
// return false; // trivial reject (i.e., *this is not within line seg's bounding box)
//Rectangle rect(*this);
//rect.inflate(resolution, resolution);
//if (!rect.isVertexInside(vtx))
// return false;
    Coord x1 = vertex0.x,
            y1 = vertex0.y;
    Coord x2 = vertex1.x,
            y2 = vertex1.y;
    Coord a = (y1 - y2),
            b = - (x1 - x2),
            c = ( x1 * (y2 - y1) - y1 * (x2 - x1) );
    Coord xba = x2 - x1,
            yba = y2 - y1;
    double lsqr = xba * xba + yba * yba,
            l = sqrt(lsqr);
    Coord yac = y1 - vtx.y,
            xac = x1 - vtx.x;
    double r = ( -yac * yba - xac * xba);
    double s = ( yac * xba - xac * yba) / l;
    return (fabs(s) < resolution && r > -resolution && r < lsqr + resolution);
}


///////////////////// MultiPoly //////////////////////
//
// Find first vertex such that vtx is within a distance 'resolution' of it.
// Output MPPos for that 'active' vertex, or invalid if none is found .
MultiPoly::MPPos MultiPoly::GetActiveVtxPos(const Vertex &vtx, Coord resolution)
{
    Rect activeArea(vtx, vtx);
    activeArea.inflate(resolution, resolution);
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
        for (VtxIt vi = pi->vtxList.begin(); vi != pi->vtxList.end(); ++vi)
            if (activeArea.isVertexInside(*vi))
                return MPPos(pi,vi);
    return MPPos();
//    return MPPos(NULL,NULL);
}

// Find first edge such that vtx is within a distance 'resolution' of it.
// Output (activePolyIter,activeVtxIter) is only valid if active edge is found, and
// refers to vertex at one end of edge. Vertex at other end of edge is
// (activePolyPos,GetNextCirc(activeVtxPos)).
// Returns true if an active edge is found .
MultiPoly::MPPos MultiPoly::GetActiveEdgePos(const Vertex &vtx, Coord resolution)
{
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi) {
        VtxIt vi = pi->vtxList.begin();
        Vertex v0 = *vi;
        for (int i = 0; i < pi->size(); ++i) {
            pi->prevCirc(vi);
            Vertex v1 = *vi;
            LineSegment l(v0, v1);
            if (l.isVertexNear(vtx, resolution))
                return MPPos(pi, vi);
            v0 = v1;
        }
    }
    return MPPos();
//    return MPPos(NULL, NULL);
}

bool MultiPoly::writePS(const char *file_name, bool bEOfill)
{
    ofstream out(file_name);
    Rect rect(boundingBox());
    out << "/poly {newpath";
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
    {
        out << "\n\t" << pi->vtxList.begin()->x << " " << pi->vtxList.begin()->y
            << " moveto\n";
        for (VtxIt vi = pi->vtxList.begin()++; vi != pi->vtxList.end(); ++vi)
            out << "\t" << vi->x << " " << vi->y << " lineto\n";
        out << "\tclosepath\n";
    }
    out << "} def\n\t16 774 translate\n\t0.8 setgray\n\tpoly "
        << (bEOfill? "eofill\n" : "fill\n")
        << "\t0 setgray\n\tpoly stroke\n\tshowpage";
    return true;
}

bool MultiPoly::write(const char *file_name)
{
    ofstream out(file_name);
    out << m_polyList.size() << "\n";
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
    {
        out << pi->vtxList.size() << " ";
        for (VtxIt vi = pi->vtxList.begin(); vi != pi->vtxList.end(); ++vi)
            out << vi->y << " " << vi->x << " ";
    }
    out << "\n";
    return true;
}
bool MultiPoly::read(const char *file_name)
{
    ifstream in(file_name);
    m_polyList.clear();
    list<Poly>::size_type nPoly(*istream_iterator<typedef list<Poly>::size_type>(in));
    for (list<Poly>::size_type p = 0; p < nPoly; ++p)
    {
        Poly poly;
        list<Vertex>::size_type nVtx(*istream_iterator<list<Vertex>::size_type>(in));
        for (list<Vertex>::size_type v = 0; v < nVtx; ++v)
        {
            Vertex vtx(*istream_iterator<Coord>(in),*istream_iterator<Coord>(in));
            poly.vtxList.push_back(vtx);
        }
        m_polyList.push_back(poly);
    }
    return true;
}
void MultiPoly::dump()
{
    ofstream out("dump.txt");
    out << m_polyList.size() << "\n";
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
    {
        out << pi->vtxList.size() << " ";
        for (VtxIt vi = pi->vtxList.begin(); vi != pi->vtxList.end(); ++vi)
            out << vi->y << " " << vi->x << " ";
    }
    out << "\n";
}

// generates a random multi polygon within the given rectangular area,
// for a given number of vertices and number of polygons
void MultiPoly::random(Rect boundRect, int nVtx, int nPoly)
{
    srand( (unsigned)time( NULL ) );
    boundRect.normalize();
    m_polyList.clear();
    for (int p = 0; p < nPoly; ++p)
    {
        Poly poly;
        for (int v = 0; v < nVtx; ++v)
        {
            Vertex vtx
                    (
                            boundRect.x0 + rand()%((int)(boundRect.x1-boundRect.x0)),
                            boundRect.y0 + rand()%((int)(boundRect.y1-boundRect.y0))
                    );
            poly.vtxList.push_back(vtx);
        }
        m_polyList.push_back(poly);
    }
}

 Rect MultiPoly::boundingBox()
{
    Rect rect(*m_polyList.begin()->vtxList.begin(), *m_polyList.begin()->vtxList.begin());
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
        for (VtxIt vi = pi->vtxList.begin(); vi != pi->vtxList.end(); ++vi)
        {
            rect.x0 = min(rect.x0, vi->x);
            rect.y0 = min(rect.y0, vi->y);
            rect.x1 = max(rect.x1, vi->x);
            rect.y1 = max(rect.y1, vi->y);
        }
    return rect;
}

 */

// finds the intersection point between two line segments and thier parametric
// values with respect to each intersecting line segment
LineSegment::IntersectionType MultiPoly::findIntersection( LineSegment &l1,
                                                           LineSegment &l2,
                                                           Vertex &intersection,
                                                           float &alpha1,
                                                           float &alpha2)
{
    LineSegment l3(l1);
    LineSegment l4(l2);
    l1.sortVertices();
    l2.sortVertices();
    Vertex vtx;
    LineSegment::IntersectionType resultType;
    if ( l1.vertex1 < l2.vertex0 ) // no x-overlap
    {
// check for y-overlap
        if ( l1 < l2 )
        {
            if (l1.ymax < l2.ymin )
                return (LineSegment::NO_INTERSECT );
        }
        else
        if (l2.ymin < l1.ymax)
            return LineSegment::NO_INTERSECT;
    }
    else
// check for adjacency
    if ( (l1.vertex0 == l2.vertex0) ||(l1.vertex1 == l2.vertex1) ||
         (l1.vertex0 == l2.vertex1) || (l1.vertex1 == l2.vertex0) )
        return ( LineSegment::NO_INTERSECT );
    resultType = l3.calcIntersection(l4, vtx, alpha1, alpha2 );
    if ( resultType == LineSegment::INTERSECT )
    {
        intersection = vtx;

        return resultType;
    }
    return (resultType);
}

// this is the method called to fill the polygon useing NZW algorithm
// finds the intersections using trivial rejections considering the
// bounding box overlap and monotonic chains
vector<Vertex> MultiPoly::findMonotone(MultiPoly &resMPoly,
                                       vector<int> &windingVector,
                                       vector<int> &directions
                                       )
{
    vector<Vertex> intersections;
    // master intersection list
    vector<Intersection> tempList;
    // the edges intersection list
    vector<Pseudovertex> p_list;
    p_list.clear();

    // phase 1- create the initial edge intersection-list and master intersection-list
    // the edge intersection list is inside the IntersectionList, each element is
    // an 'edge', and each edge contains a list of intersections of this edge.
    // this might seem confusing, but it is what it is.
    // to me, it seems, the first element is an edge vertex/intersection, and the next
    // elements are it's intersections linked list (implemented weirdly).
    //
    // we go over all polygons and create:
    // 1. a single polygon edge list (that will also contain the edge intersections)
    // 2. insert the poly vertex as initial intersection in the master list
    //
    // note:: don't get confused between the edge intersection lists and the master intersection list.
    //
    for ( PolyIt pii = m_polyList.begin(); pii != m_polyList.end(); ++pii)
    {
        VtxIt vit0, vit1;
        for (VtxIt vi = pii->vtxList.begin(); vi != pii->vtxList.end();++vi)
        {
            Pseudovertex pseudo;
            vit0 = vi;
            vit1 = vi;

            pii->prevCirc(vit0);
            pii->nextCirc(vit1);

            LineSegment l1(*vit0, *vi);
            LineSegment l2(*vi, *vit1);

            l2.sortVertices();

            nVertex i(*vi, l2);

            tempList.push_back(Intersection ( *vi, l2, l1 ) );
            // first element is the edge vertex
            pseudo.ilist.push_back(i);
            p_list.push_back( pseudo );
        }
    }

    // the edges intersection list
    IntersectionList ivList(p_list);

    // add the intersection points to the list, as Intersection objects
    // phase 2: for each polygon, find intersection among it's own edges
    // and insert them into master intersection list
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
    {
        VtxIt vi0 = pi->vtxList.begin(), vi1;
        unsigned int x_monotone=0;
        bool reached = false;
        vi1 = vi0;
        VtxIt start = vi0;
        ++vi1;
        //find intersection points for every polygon.
        while ( vi1 != pi->vtxList.end() && (reached == false))
        {
            LineSegment l1(*vi0, *vi1);
            vi0 = vi1;
            ++vi1;
            if ( vi1 == pi->vtxList.end() )
            {
                vi1 = start;
                reached = true ;
            }
            LineSegment l2(*vi0, *vi1);
            LineSegment temp(l2 );
            VtxIt walker, walker1;
            walker = start;
            walker1 = walker;
            ++walker1;
            // walk thru the list from the beginning to find
            // intersection with the non-monotone edge
            while ( walker1 != vi0 )
            {
                LineSegment l1(*walker, *walker1 );
                l2 = temp;
                Vertex intersection;
                float al1, al2;
                if ( findIntersection(l1,l2,intersection, al1, al2)
                     == LineSegment::INTERSECT )
                {
                    intersections.push_back(intersection);
                    Vertex searchVertex1, searchVertex2;
                    if (l1.m_swappedVertices)
                        searchVertex1 = l1.vertex1;
                    else
                        searchVertex1 = l1.vertex0;
                    if ( l2.m_swappedVertices)
                        searchVertex2 = l2.vertex1;
                    else
                        searchVertex2 = l2.vertex0;
                    tempList.push_back( Intersection ( intersection, searchVertex1,
                                                       searchVertex2, al1, al2, l1, l2 ) );
                }
                walker= walker1;
                ++walker1;
            }
        }
    }

    // phase 3:: find intersections of polygons between themselves and
    // add the intersections into the master list, this uses bounding boxes optimizations
    vector<Vertex> resultIntersections = findIntersections(tempList);


    // intersections and resultIntersections are redundant, they are
    // just used for debugging
    for ( unsigned int i=0; i < resultIntersections.size() ; i++ )
        intersections.push_back(resultIntersections[i]);


    sort ( ivList.p_list.begin(), ivList.p_list.end() );

    // a function to fill polygon edge array(ivList) with the indices from the
    // intersection master list.
    // after most of the intersection master list, we use it to fill index information
    // for the edge array list, and then we use it again to fill pointer data in the
    // master list
    fillAddress(ivList, tempList);

    // now, we have a complete master list, we can traverse it for polygons.
    polygonPartition( resMPoly,tempList,windingVector, directions);

    return intersections;
}

// finds the intersection points between every polygon edge
vector<Vertex> MultiPoly::findIntersections(vector<Intersection> &tempList)
{
    vector<LineSegment> edges, edges1;
    vector<Vertex> intersections;
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
    {
        PolyIt pi0 = pi;
        VtxIt vi0 = pi0->vtxList.begin(), vi1;
        unsigned nVtx = pi0->vtxList.size();
        for (unsigned i = 0; i < nVtx; ++i)
        {
            vi1 = vi0;
            pi0->nextCirc(vi1);
            Vertex v0(*vi0),v1(*vi1);
            LineSegment edge(*vi0, *vi1);
            edge.sortVertices();
            edges.push_back(edge);
            vi0 = vi1;
        }
        sort(edges.begin(), edges.end());
        ++pi0;
        while ( pi0 != m_polyList.end() )
        {
            VtxIt vi2 = pi0->vtxList.begin(), vi3;
            unsigned nVtx = pi0->vtxList.size();
            for (unsigned i = 0; i < nVtx; ++i)
            {
                vi3 = vi2;
                pi0->nextCirc(vi3);
                Vertex v0(*vi2),v1(*vi3);
                LineSegment edge(*vi2, *vi3);

                edge.sortVertices();
                edges1.push_back(edge);
                vi2 = vi3;
            }
            sort(edges1.begin(), edges1.end());
            // make vector of intersections
            unsigned nEdges = edges.size();
            unsigned nEdges1 = edges1.size();
            if (nEdges > 2)
            {
                for (unsigned i = 0; i < nEdges; ++i)
                    for (unsigned j = 0; j < nEdges1; ++j)
                    {
                        LineSegment li, lj;
                        if ( !edges[i].m_swappedVertices )
                            li = edges[i];
                        else
                        {
                            li.vertex0 = edges[i].vertex1;
                            li.vertex1 = edges[i].vertex0;
                        }
                        if ( !edges1[j].m_swappedVertices )
                            lj = edges1[j];
                        else
                        {
                            lj.vertex0 = edges1[j].vertex1;
                            lj.vertex1 = edges1[j].vertex0;
                        }
                        // Because of sort above: edges[i].rect.x0 <= edges[j].rect.x0
                        // if bounding boxes have x-overlap
                        if (edges1[j].vertex0.x < edges[i].vertex1.x)
                        {
                            // check for y-overlap of bounding boxes
                            if (edges[i].ymin < edges1[j].ymin)
                            {
                                if (edges[i].ymax <= edges1[j].ymin) continue;
                            }
                            else
                            if (edges1[j].ymax <= edges[i].ymin) continue;
                            // Check if both left [right] ends are not coincident (i.e.,
                            // valid intersection)
                            if(edges[i].vertex0!=edges1[j].vertex0  &&  edges[i].vertex1!= edges1[j].vertex1)
                            {
                                Vertex intersection;
                                float param1, param2;
                                if (li.calcIntersection(lj, intersection, param1, param2)
                                    == LineSegment::INTERSECT)
                                {
                                    intersections.push_back(intersection);
                                    Vertex searchVertex1, searchVertex2;
                                    if (edges[i].m_swappedVertices)
                                        searchVertex1 = edges[i].vertex1;
                                    else
                                        searchVertex1 = edges[i].vertex0;
                                    if ( edges1[j].m_swappedVertices)
                                        searchVertex2 = edges1[j].vertex1;
                                    else
                                        searchVertex2 = edges1[j].vertex0;
                                    li.sortVertices();
                                    lj.sortVertices();
                                    tempList.push_back( Intersection ( intersection,
                                                                       searchVertex1, searchVertex2, param1, param2,
                                                                       li , lj) );
                                }
                            }
                        }

                    }
            }
            ++pi0;
        }// end of while for pi0 counter
    }// end of for
    return intersections;
}

// a function to fill polygon edge array(ivList) with the indices from the
// intersection master list
void MultiPoly::fillAddress(IntersectionList &ivList, vector<Intersection> &interVector)
{
    sort(interVector.begin() , interVector.end());

    for ( int i =0; i < interVector.size() ; i ++ )
    {
        if ( (interVector[i].param1 != 0 ) && (interVector[i].param2 != 0 ) )
        {
            int i11 = 0, i21 = 0;
            Vertex searchVertex1 = interVector[i].origin1;
            Vertex searchVertex2 = interVector[i].origin2;

            // zero index is always a concrete polygon vertex or an intersection,
            // but always concrete part of the polygon
            while ( ivList.p_list[i11].ilist[0].v != searchVertex1)
                i11++;
            while ( ivList.p_list[i21].ilist[0].v != searchVertex2)
                i21++;

            nVertex Inter1(interVector[i].v, interVector[i].param1, i,interVector[i].l1);
            nVertex Inter2(interVector[i].v, interVector[i].param2, i,interVector[i].l2);

            ivList.p_list[i11].ilist.push_back(Inter1);
            ivList.p_list[i21].ilist.push_back(Inter2);
        }
        else
        {
            int vtxIt = 0;
            Vertex vtx = interVector[i].v;
            while ( ivList.p_list[vtxIt].ilist[0].v != vtx )
                vtxIt++;
            ivList.p_list[vtxIt].ilist[0].index = i;
        }
    }

    // sort ONLY the intersections of the poly edge array
    for ( PseudoIt ps = ivList.p_list.begin(); ps != ivList.p_list.end(); ++ps)
        sort ( ps->ilist.begin() + 1, ps->ilist.end() );

    fillIndices( ivList, interVector );
}

// A method to fill in the last node of polygon edge array (ivList), which points to
// the ending vertex of the line
// setting up the last Vertex that points to the starting vertex of the
// intersecting and setting up its corresponding index in the intersection
// master list (interVector)
void MultiPoly::fillIndices (IntersectionList &ivList, vector<Intersection> &interVector)
{
    PseudoIt ps;
    for (ps = ivList.p_list.begin(); ps != ivList.p_list.end(); ++ps)
    {
        int vtxIt = 0;
        Vertex searchVtx;
        LineSegment ls( (ps->ilist.end() - 1)->l );
        if ( ls.m_swappedVertices)
            searchVtx = ls.vertex0;
        else
            searchVtx = ls.vertex1;
        while ( interVector[vtxIt].v != searchVtx )
            vtxIt++;
        ps->ilist.push_back( nVertex( searchVtx, vtxIt,(ps->ilist.end() - 1)->l ) ) ;
    }

    // setting up the indices for the intersections in the interVector
    for ( ps = ivList.p_list.begin(); ps != ivList.p_list.end(); ++ps) {
        for (int i= 0; i < ps->ilist.size()-1; ++i)
        {
            nVertex tempVtx = ps->ilist[i+1];
            int tempIndex = ps->ilist[i].index;
            if ( interVector[tempIndex].origin2 == ps->ilist[0].v )
                interVector[tempIndex].index2 = tempVtx.index;
            else
            if ( interVector[tempIndex].origin1 == ps->ilist[0].v )
                interVector[tempIndex].index1 = tempVtx.index;
            else
            if ( interVector[tempIndex].v == ps->ilist[0].v )
                interVector[tempIndex].index1 = tempVtx.index;
        }
    }

    // setting up the self-index of each interseciton object
    for ( int i =0 ; i < interVector.size() ; i ++ )
        interVector[i].selfIndex = i;
}

// traverse from the left most vertex of the ivList and form the queue of subsequent
// polygons done until the queue becomes empty i.e all the vertices of the
// multipolygon has been processed
// static method, depends only on it's inputm that means on the intersection master list
void MultiPoly::polygonPartition(MultiPoly &resMPoly,
                                 const vector<Intersection> &tempList,
                                 vector<int> &windingVector,
                                 vector<int> &directions
                                 )
{
    vector<Intersection> interVector;
    PolyIt pIt;
    bool finished = false;
    Vertex startVtx, currVtx;
    Intersection prevInter;
    interVector = tempList; // todo:: this is a waste
    bool foundWinding = false;
    // todo:: interVector and tempList are mostly immutable so we can use indices
    // todo:: or pointers instead of objects
    stack<Intersection> interStack;
    MultiPoly resMP;
    vector<int> wVector;
    int currDirection =0;

    do
    {
        int index =0, currIndex=0;// index of interVector
        Poly currPoly;

        if ( !interStack.empty() )
        {
            // remove intersections that dont have a next intersection object from the head of queue
            while ( !interStack.empty() )
            {
                int idx = interStack.top().selfIndex;
                if ( interVector[idx].index1 == -1 && interVector[idx].index2 == -1)
                    interStack.pop();
                else
                    break;
            }
        }

        if ( !interStack.empty() && interStack.top().selfIndex > 0)
            index= interStack.top().selfIndex;
        else
        {
            // in case the algorithm operates on for example two components,
            // that are not connected, therefore were not discoverable in the stack,
            // then we need to look ahead in the master intersection list for a next candidate.
            //
            // we couldn't find a promising index from the stack, therefore
            // we go over the intersection vector from below to top
            // to find the first index that leads somewhere, i.e has
            // a next intersection point
            //
            // the first time we visit, then the first index we find is a convex
            // vertex, because the intersection list was sorted lexically by (x, y)
            //
            // todo: this might be optimizes by recording the last position of
            // todo: valid index (index that satisfies the condition in the loop)
            // todo: this index should be monotone, though I guess it is not performing
            // todo: a lot of work now, it just compares things
            index=0;
            while(( interVector[index].index1 == -1)
                  && ( interVector[index].index2 == -1 ) &&(index < interVector.size()) )
            {
                index++;
            }
        }

        // if we reached the last intersection, we are done
        // else, this is out current index for work
        if ( index >= interVector.size()-1 )
            finished = true;
        else
            currIndex = index;


        // start a new polygon vertex walk
        if ( !finished )
        {
            int currWinding = 0;

            startVtx = interVector[currIndex].v;
            currVtx = interVector[currIndex].v;
            prevInter = interVector[currIndex];

//            /*
            const int startIndex = currIndex;
            int firstIndex = startIndex;

            // this is for winding caclulations
            // fix vertex to find the winding number before changing the index values!!
            Vertex vtx;
            if ( interVector[startIndex].index1 == -1 )
            {
                // todo:: get original starting vertex of segment, this can be simplified
                if ( interVector[startIndex].l1.m_swappedVertices)
                    vtx = interVector[startIndex].l1.vertex1;
                else
                    vtx = interVector[startIndex].l1.vertex0;
            }
            else
            {
                // todo:: get original starting vertex of segment, this can be simplified
                if ( interVector[startIndex].l2.m_swappedVertices)
                    vtx = interVector[startIndex].l2.vertex1;
                else
                    vtx = interVector[startIndex].l2.vertex0;
            }

            if ( interVector[startIndex].index1 == -1 || interVector[startIndex].index2== -1 )
                currWinding = interVector[startIndex].winding;

            // end winding setup
//             */

            currPoly.vtxList.push_back( currVtx);
            interStack.push(interVector[currIndex]);

            // walk the polygon until we get back to the start point,
            // on the way, push potential companion vertices into the stack
            do {
                // find the next vertex to visit and tag visited companion vertices
                if (( interVector[currIndex].index1 == -1) && ( interVector[currIndex].index2 == -1 ) )
                    currIndex++;
                else
                {
                    if ( ( interVector[currIndex].origin1 == currVtx )
                         && ( interVector[currIndex].index2 == -1 ) )
                    {
                        int tempIndex = interVector[currIndex].index1;
                        prevInter = interVector[currIndex];
                        interVector[currIndex].index1 = -1;
                        currIndex = tempIndex;
                    }
                    else
                    {
                        if ( (( interVector[currIndex].origin1 == prevInter.origin1)
                              ||( interVector[currIndex].origin1 == prevInter.origin2 ))
                             && ( interVector[currIndex].index2 != -1 ) )
                        {
                            int tempIndex = interVector[currIndex].index2;
                            prevInter = interVector[currIndex];
                            interVector[currIndex].index2 = -1;
                            currIndex = tempIndex;
                        }
                        else
                        {
                            int tempIndex = interVector[currIndex].index1;
                            prevInter = interVector[currIndex];
                            interVector[currIndex].index1 = -1;
                            currIndex = tempIndex;
                        }
                    }
                }

                if (currIndex > -1)
                {
                    currVtx = interVector[currIndex].v;
                    currPoly.vtxList.push_back( currVtx);
                    interStack.push(interVector[currIndex]);

//                    /*
                    // this part is only for resolving winding
                    if ( !foundWinding)
                    {
                        if (firstIndex == startIndex)
                            firstIndex = currIndex;
                        else
                        {
                            const int secondIndex = currIndex;
                            foundWinding = true;
                            Intersection temp1,temp2;
                            temp1 = interVector[firstIndex];
                            temp2 = interVector[secondIndex];
                            // tests if this is intersection is a T intersection |-
                            if ((( interVector[startIndex].v.x
                                   == interVector[startIndex].origin1.x)
                                 && ( interVector[startIndex].v.y
                                      == interVector[startIndex].origin1.y ))
                                || (( interVector[startIndex].v.x
                                      == interVector[startIndex].origin2.x)
                                    && ( interVector[startIndex].v.y
                                         == interVector[startIndex].origin2.y )))
                            {
                                float win1;
                                win1 = xProd(interVector[startIndex].l2.vertex0,
                                             interVector[startIndex].v, interVector[firstIndex].v);
                                if (win1 > 0 )
                                {
                                    currDirection = 0;
                                    currWinding--;
                                }
                                else
                                {
                                    currDirection = 1;
                                    currWinding++;
                                }
                                interVector[startIndex].direction = currDirection;
                            }
                            else
                            {
                                float win;
                                win = xProd(vtx, interVector[startIndex].v,
                                            interVector[firstIndex].v);
                                if (win > 0 )
                                {
                                    currDirection = 0;
                                    if ( interVector[startIndex].direction != currDirection)
                                        currWinding= currWinding - 2;
                                    else
                                        currWinding--;
                                }
                                else
                                {
                                    currDirection = 1;
                                    if ( interVector[startIndex].direction != currDirection)
                                        currWinding= currWinding + 2;
                                    else
                                        currWinding++;
                                }
                            }
                            interVector[startIndex].direction = currDirection;
                            interVector[firstIndex].direction = currDirection;
                            interVector[secondIndex].direction = currDirection;
                            interVector[startIndex].winding = currWinding;
                            interVector[firstIndex].winding = currWinding;
                            interVector[secondIndex].winding = currWinding;
                        }
                    }
                    else
                    {
                        interVector[currIndex].winding = currWinding;
                        interVector[currIndex].direction = currDirection;
                    }
//                     */
                }
            } while ( (currVtx != startVtx) && ( currIndex > -1 ) ) ;
            wVector.push_back(currWinding);
            directions.push_back(currDirection);
            foundWinding = false;
            resMP.m_polyList.push_back(currPoly);
        } ;
    } while ( !finished );

    resMPoly = resMP;
    windingVector = wVector;
}

MultiPoly::MultiPoly(const MultiPoly &mp) : m_polyList(mp.m_polyList) {}

float MultiPoly::xProd(const Vertex &p, Vertex &q, Vertex &r) {
    return ( (q.x * ( r.y - p.y )) + (p.x*( q.y - r.y )) + (r.x * ( p.y - q.y )) );
}
