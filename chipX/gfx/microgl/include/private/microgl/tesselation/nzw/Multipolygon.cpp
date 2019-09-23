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

Intersection::Intersection( Vertex *vtx, float p1, float p2, const LineSegment &li, const LineSegment &lj) {
    v = vtx;
    index1 = -1;
    index2 = -1;
    param1 = p1;
    param2 = p2;
    l1 = li;
    l2 = lj;
}

Intersection::Intersection( Vertex *vtx, const LineSegment &li, const LineSegment &lj) {
    v = vtx;
    index1 = -1;
    index2 = -1;
    param1 = 0;
    param2 = 0;
    l1 = li;
    l2 = lj;
    winding = 0;
    direction = 0;
    selfIndex = 0;
}

bool Intersection::operator<(const Intersection &i) const { return (*v < *i.v ); }

bool Intersection::operator<=(const Intersection &i) const { return (*v <= *i.v ); }

void Intersection::operator=(const Intersection &i) {
    v = i.v;
    index1 = i.index1;
    index2= i.index2;
    param1 = i.param1;
    param2 = i.param2;
    l1 = i.l1;
    l2 = i.l2;
    winding = i.winding;
    direction = i.direction;
    selfIndex = i.selfIndex;
}


nVertex::nVertex() {}

nVertex::nVertex( Vertex *vtx, float p, int i) {
    v = vtx;
    param = p;
    index = i;
}


void nVertex::operator=(const nVertex &i) {
    v = i.v;
    param= i.param;
    index = i.index;
}

bool nVertex::operator<(const nVertex &n) const { return (param < n.param ); }

bool nVertex::operator<=(const nVertex &i) const { return (param <= i.param ); }

Pseudovertex::Pseudovertex() {}

Pseudovertex::Pseudovertex(const nVertex &n) { ilist.push_back(n); }

Pseudovertex::Pseudovertex(const Pseudovertex &p) : ilist(p.ilist) {}

bool Pseudovertex::operator<(const Pseudovertex &s) const { return ( *(ilist.begin()->v) < *(s.ilist.begin()->v) ); }

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
    auto * l0_vertex0 = this->start();
    auto * l0_vertex1 = this->end();

    auto * l1_vertex0 = l.start();
    auto * l1_vertex1 = l.end();

    float dx21 = l0_vertex1->x - l0_vertex0->x, dy21 = l0_vertex1->y - l0_vertex0->y,
            dx43 = l1_vertex1->x - l1_vertex0->x, dy43 = l1_vertex1->y - l1_vertex0->y;
    float dem = dx21 * dy43 - dy21 * dx43;
    // parallel lines
    if (fabs(dem) < NOISE)
        return PARALLEL;
    else {
        float dx12 = l0_vertex0->x - l1_vertex0->x,
                dy12 = l0_vertex0->y - l1_vertex0->y;
        float dx = l1_vertex0->x - l0_vertex0->x,
                dy = l1_vertex0->y - l0_vertex0->y;
        alpha = (dy12*dx43 - dx12*dy43)/dem;
        alpha1 = ( dy21*dx - dx21*dy)/dem;
        // The intersecting point
        intersection.x = l0_vertex0->x + dx21 * alpha;
        intersection.y = l0_vertex0->y + dy21 * alpha;
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
    m_swappedVertices = !(*vertex0 < *vertex1);
    if (m_swappedVertices)
        swap(vertex0, vertex1);
}

LineSegment::LineSegment(Vertex *vtx0, Vertex *vtx1) : vertex0(vtx0), vertex1(vtx1) {}

bool LineSegment::operator<(const LineSegment &ls) const {
    return *vertex0 < *(ls.vertex0);
}

// this is the method called to fill the polygon useing NZW algorithm
// finds the intersections using trivial rejections considering the
// bounding box overlap and monotonic chains
void MultiPoly::findMonotone(MultiPoly &resMPoly,
                                       vector<int> &windingVector,
                                       vector<int> &directions
)
{
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
//    for ( PolyIt pii = m_polyList.begin(); pii != m_polyList.end(); ++pii){
    for (unsigned long poly = 0; poly < m_polyList.size(); ++poly) {
//
        auto & current_list = m_polyList[poly].vtxList;
        const auto size = current_list.size();

//        VtxIt vit0, vit1;
        for (unsigned long ix = 0; ix < size; ++ix) {
//        for (VtxIt vi = pii->vtxList.begin(); vi != pii->vtxList.end();++vi) {
//            /*
            int ix_next = ix+1 >= size ? 0 : ix+1;
            int ix_prev = int(ix-1) < 0 ? size-1 : ix-1;
            Vertex * current = &current_list[ix];
            Vertex * next = &current_list[ix_next];
            Vertex * prev = &current_list[ix_prev];

            LineSegment l1(prev, current);
            LineSegment l2(current, next);
            Pseudovertex pseudo;
            l1.sortVertices();
            l2.sortVertices();

            nVertex i1(current, 0, -1);
            nVertex i2(next, 2, -1);

            tempList.push_back(Intersection(current, 1, 0, l1, l2 ));
//             first element is the edge vertex
            pseudo.ilist.push_back(i1);
//             last vertex of the edge
            pseudo.ilist.push_back(i2);
//
            p_list.push_back( pseudo );
//*/

            /*
            vit0 = vi;
            vit1 = vi;

            pii->prevCirc(vit0);
            pii->nextCirc(vit1);
            Pseudovertex pseudo;

            LineSegment l1(&(*vit0), &(*vi));
            LineSegment l2(&(*vi), &(*vit1));
            l2.sortVertices();

            // first vertex of the edge
            nVertex i1(&(*vi), 0, -1);
            nVertex i2( &(*vit1), 2, -1);

            tempList.push_back(Intersection ( &(*vi), 1, 0, l1, l2 ) );
            // first element is the edge vertex
            pseudo.ilist.push_back(i1);
            // last vertex of the edge
            pseudo.ilist.push_back(i2);

            p_list.push_back( pseudo );

             */
        }
    }

    // the edges intersection list
    IntersectionList ivList(p_list);

    // phase 2:: find self intersections of each polygon
    for (unsigned long poly = 0; poly < m_polyList.size(); ++poly)
    {
        auto & current_list = m_polyList[poly].vtxList;
        const unsigned long current_list_size = current_list.size();

        if(current_list.empty())
            continue;

        for (unsigned long ix = 0; ix < current_list_size-1; ++ix) {

            LineSegment edge_0{&current_list[ix], &current_list[(ix+1)]};

            for (unsigned long jx = ix+1; jx < current_list_size; ++jx) {
                LineSegment edge_1{&current_list[jx],
                                   &current_list[(jx+1)%current_list_size]};

                Vertex intersection;
                float al1, al2;

                // see if any of the segments have a mutual endpoint
                if(!edge_1.is_bbox_overlapping_with(edge_0))
                    continue;
                if (edge_1.has_mutual_endpoint(edge_0))
                    continue;
                if(edge_1.calcIntersection(edge_0, intersection, al1, al2)
                                                        !=LineSegment::INTERSECT)
                    continue;

                auto * found_intersection = new Vertex(intersection);

                v_interesections.push_back(found_intersection);

                tempList.push_back(Intersection(found_intersection,
                        al1, al2, edge_1, edge_0));

            }
        }

    }

    // phase 3:: find intersections of polygons between themselves and
    // add the intersections into the master list, this uses bounding boxes optimizations
    findIntersections(tempList);

    sort ( ivList.p_list.begin(), ivList.p_list.end() );

    fillAddress(ivList, tempList);

    // now, we have a complete master list, we can traverse it for polygons.
    polygonPartition( resMPoly,tempList,windingVector, directions);
}

// finds the intersection points between every polygon edge
vector<Vertex *> MultiPoly::findIntersections(vector<Intersection> &tempList)
{
    vector<LineSegment> edges, edges1;
//    vector<Vertex> intersections;
//    const index_t size = m_polyList.size();

//    for (index_t poly = 0; poly < size; ++poly)
    for (PolyIt pi = m_polyList.begin(); pi != m_polyList.end(); ++pi)
    {
        edges.clear();
        PolyIt pi0 = pi;
        VtxIt vi0 = pi0->vtxList.begin(), vi1;
        unsigned nVtx = pi0->vtxList.size();
        for (unsigned i = 0; i < nVtx; ++i)
        {
            vi1 = vi0;
            pi0->nextCirc(vi1);
            Vertex v0(*vi0),v1(*vi1);
            LineSegment edge(&(*vi0), &(*vi1));
            edge.sortVertices();
            edges.push_back(edge);
            vi0 = vi1;
        }
        sort(edges.begin(), edges.end());
        ++pi0;
        while ( pi0 != m_polyList.end() )
        {
            edges1.clear();
            VtxIt vi2 = pi0->vtxList.begin(), vi3;
            unsigned nVtx = pi0->vtxList.size();
            for (unsigned i = 0; i < nVtx; ++i)
            {
                vi3 = vi2;
                pi0->nextCirc(vi3);
                Vertex v0(*vi2),v1(*vi3);
                LineSegment edge(&(*vi2), &(*vi3));

                // we have to sort them on x-axis for the bigger sort later
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
                for (unsigned i = 0; i < nEdges; ++i) {
                    auto &edge_0 = edges[i];

                    for (unsigned j = 0; j < nEdges1; ++j) {

                        auto &edge_1 = edges1[j];

                        // if edge_1 is completely to the right of edge_0, then no intersection
                        // occurs, and also, since edge_1 syblings are sorted on the x-axis,
                        // we can skip it's upcoming syblings, therefore we break;
                        auto h_classify = edge_1.classify_horizontal(edge_0);
                        if (h_classify == LineSegment::bbox_axis::end_of)
                            break;
                        else if (h_classify != LineSegment::bbox_axis::overlaps)
                            continue;
                        else {
                            // we have horizontal overlap, let's test for vertical overlap
                            if (edge_1.classify_vertical(edge_0) != LineSegment::bbox_axis::overlaps)
                                continue;
                        }

                        // see if any of the segments have a mutual endpoint
                        if (edge_1.has_mutual_endpoint(edge_0))
                            continue;

                        Vertex intersection;
                        float param1, param2;

                        // test and compute intersection
                        if (edge_0.calcIntersection(edge_1, intersection, param1, param2)
                            != LineSegment::INTERSECT)
                            continue;

                        auto *found_intersection = new Vertex(intersection);

                        v_interesections.push_back(found_intersection);

                        tempList.push_back(Intersection(found_intersection,
                                                        param1, param2,
                                                        edge_0, edge_1));

                    }

                }

            }

            ++pi0;
        }

    }

    return v_interesections;
}

// a function to fill polygon edge array(ivList) with the indices from the
// intersection master list.
// after most of the intersection master list, we use it to fill index information
// for the edge array list, and then we use it again to fill pointer data in the
// master list
void MultiPoly::fillAddress(IntersectionList &ivList, vector<Intersection> &interVector)
{
    sort(interVector.begin() , interVector.end());

    for ( int i =0; i < interVector.size() ; i ++ )
    {
        // push real intersection into the polygon edges list, for each edge push
        // it's intersecting edges, thus at each edge create a linked list
        // of intersections, later ont we will sort them
        // COMPARE AS POINTERS, VERY IMPORTANT !!!!
        if ( (interVector[i].param1 != 0 ) && (interVector[i].param2 != 0 ) )
        {
            int i11 = 0, i21 = 0;
            Vertex *searchVertex1 = interVector[i].origin1();
            Vertex *searchVertex2 = interVector[i].origin2();

            // zero index is always a concrete polygon vertex ,
            // but always concrete part of the polygon, thus can be identified with
            // unique id
            while ( ivList.p_list[i11].ilist[0].v != searchVertex1)
                i11++;
            while ( ivList.p_list[i21].ilist[0].v != searchVertex2)
                i21++;

            nVertex Inter1(interVector[i].v, interVector[i].param1, i);
            nVertex Inter2(interVector[i].v, interVector[i].param2, i);

            ivList.p_list[i11].ilist.push_back(Inter1);
            ivList.p_list[i21].ilist.push_back(Inter2);
        }
        else
        {
            // natural polygon vertex joints can be filled with indices already
            // this are the first vertex and last vertex, that define an edge of a polygon.
            // first vertex is the start vertex, and second one is the end vertex.
            // other vertices are in between, later on we will sort them
            int vtxIt = 0;
            Vertex *vtx = interVector[i].v;
            while ( ivList.p_list[vtxIt].ilist[0].v != vtx )
                vtxIt++;
            ivList.p_list[vtxIt].ilist[0].index = i;

            vtxIt = 0;
            while ( ivList.p_list[vtxIt].ilist[1].v != vtx )
                vtxIt++;
            ivList.p_list[vtxIt].ilist[1].index = i;
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
    // go over the edges and add the last vertex to each edge
    // COMPARE AS POINTERS, VERY IMPORTANT !!!!
    PseudoIt ps;

    // go over on the edges list (this may represent a polygon if there is one component
    // given to the algorithm)
    // setting up the indices for the intersections in the interVector,
    // this is what we worked for
    for ( ps = ivList.p_list.begin(); ps != ivList.p_list.end(); ++ps) {
        // go over the intersections of each edge
        for (int i= 0; i < ps->ilist.size()-1; ++i)
        {
            nVertex tempVtx = ps->ilist[i+1];
            int tempIndex = ps->ilist[i].index;

            // this is actually very simple. each edge is broken/ represented as
            // segments *--*--*-*----* etc..
            // for each point on the edge, update the master list where do we
            // go next.
            // I think we may avoid the third condition, in case we record
            // polygon vertices with origin 2 as well. currently the code does not,
            // no idea why
            if ( interVector[tempIndex].origin2() == ps->ilist[0].v )
                interVector[tempIndex].index2 = tempVtx.index;
            else if ( interVector[tempIndex].origin1() == ps->ilist[0].v )
                interVector[tempIndex].index1 = tempVtx.index;
            else if ( interVector[tempIndex].v == ps->ilist[0].v )
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
    Vertex *startVtx, *currVtx;
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
            Vertex *vtx;
            if ( interVector[startIndex].index1 == -1 )
                vtx = interVector[startIndex].origin1();
            else
                vtx = interVector[startIndex].origin2();

            /*
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
             */


            if ( interVector[startIndex].index1 == -1 || interVector[startIndex].index2== -1 )
                currWinding = interVector[startIndex].winding;

            // end winding setup
//             */

            currPoly.vtxList.push_back( *currVtx);
            interStack.push(interVector[currIndex]);

            // walk the polygon until we get back to the start point,
            // on the way, push potential companion vertices into the stack
            do {
                // find the next vertex to visit and tag visited companion vertices
                if (( interVector[currIndex].index1 == -1) && ( interVector[currIndex].index2 == -1 ) )
                    currIndex++;
                else
                {
                    if ( ( interVector[currIndex].origin1() == currVtx )
                         && ( interVector[currIndex].index2 == -1 ) )
                    {
                        int tempIndex = interVector[currIndex].index1;
                        prevInter = interVector[currIndex];
                        interVector[currIndex].index1 = -1;
                        currIndex = tempIndex;
                    }
                    else
                    {
                        if ( (( interVector[currIndex].origin1() == prevInter.origin1())
                              ||( interVector[currIndex].origin1() == prevInter.origin2() ))
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
                    currPoly.vtxList.push_back( *currVtx);
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
                            if ((( interVector[startIndex].v->x
                                   == interVector[startIndex].origin1()->x)
                                 && ( interVector[startIndex].v->y
                                      == interVector[startIndex].origin1()->y ))
                                || (( interVector[startIndex].v->x
                                      == interVector[startIndex].origin2()->x)
                                    && ( interVector[startIndex].v->y
                                         == interVector[startIndex].origin2()->y )))
                            {
                                float win1;
                                // todo:: might be problematic
//                                win1 = xProd(*interVector[startIndex].l2.vertex0,
                                win1 = xProd(*interVector[startIndex].l2.start(),
                                             *interVector[startIndex].v, *interVector[firstIndex].v);
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
                                win = xProd(*vtx, *interVector[startIndex].v,
                                            *interVector[firstIndex].v);
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
