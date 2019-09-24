#include <microgl/tesselation/nzw/simplify_components.h>

bool compare_vertices(const Vertex &v1, const Vertex &v2) {
    return v1.x<v2.x || (v1.x==v2.x && v1.y < v2.y);
}


intersection::intersection(Vertex *vtx, float p1, float p2, const segment &li, const segment &lj) {
    v = vtx;
    index1 = -1;
    index2 = -1;
    param1 = p1;
    param2 = p2;
    l1 = li;
    l2 = lj;
}

bool intersection::operator<(const intersection &i) const {
    return compare_vertices(*v, *i.v);
}

edge_vertex::edge_vertex(Vertex *vtx, float p, int i) {
    v = vtx;
    param = p;
    index = i;
}

bool edge_vertex::operator<(const edge_vertex &n) const { return (param < n.param ); }

bool edge::operator<(const edge &s) const {
    return compare_vertices(*(vertices.begin()->v), *(s.vertices.begin()->v) );
//    return ( *(ilist.begin()->v) < *(s.ilist.begin()->v) );
}

Poly::Poly(const Poly &p) : vtxList(p.vtxList){}


int Poly::size() const {
    return vtxList.size();
}

const float segment::NOISE = 1e-5f;

segment::IntersectionType segment::calcIntersection
        (const segment &l, Vertex &intersection, float &alpha, float &alpha1)
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

void segment::sortVertices() {
//    m_swappedVertices = !(*vertex0 < *vertex1);
    m_swappedVertices = !(compare_vertices(*vertex0, *vertex1));
    if (m_swappedVertices)
        swap(vertex0, vertex1);
}

segment::segment(Vertex *vtx0, Vertex *vtx1) : vertex0(vtx0), vertex1(vtx1) {}


bool segment::operator<(const segment &ls) const {
    return compare_vertices(*vertex0, *(ls.vertex0));
//    return *vertex0 < *(ls.vertex0);
}

// this is the method called to fill the polygon useing NZW algorithm
// finds the intersections using trivial rejections considering the
// bounding box overlap and monotonic chains
void MultiPoly::findMonotone(MultiPoly &resMPoly,
                             vector<int> &windingVector,
                             vector<int> &directions
)
{
    master_intersection_list master_list;
    edge_list edges;

    // phase 1:: fill the edges structure and initial intersections
    // in the master list
    for (unsigned long poly = 0; poly < m_polyList.size(); ++poly) {
        auto & current_list = m_polyList[poly].vtxList;
        const auto size = current_list.size();

        for (unsigned long ix = 0; ix < size; ++ix) {
            int ix_next = ix+1 >= size ? 0 : int(ix+1);
            int ix_prev = int(ix-1) < 0 ? int(size-1) : int(ix-1);
            Vertex * current = &current_list[ix];
            Vertex * next = &current_list[ix_next];
            Vertex * prev = &current_list[ix_prev];

            segment l1(prev, current);
            segment l2(current, next);
            edge edge;
            l1.sortVertices();
            l2.sortVertices();

            edge_vertex i1(current, 0, -1);
            edge_vertex i2(next, 2, -1);
            // report vertex as a vertex intersection in the master list
            master_list.push_back(intersection(current, 1, 0, l1, l2 ));
            // first element is the edge vertex
            edge.vertices.push_back(i1);
            // last vertex of the edge
            edge.vertices.push_back(i2);

            edges.push_back( edge );
        }
    }

    // phase 3:: find intersections of polygons among other polygons and
    // add the intersections into the master list, this uses bounding boxes optimizations
    findIntersections(master_list);
    // use the edge structure to fill the master list
    fillAddress(edges, master_list);
    // now, we have a complete master list, we can traverse it for polygons.
    polygonPartition(resMPoly, master_list, windingVector, directions);
}

vector<Vertex *> MultiPoly::findIntersections(master_intersection_list &master_list)
{
    // phase 2:: find self intersections of each polygon
    for (unsigned long poly = 0; poly < m_polyList.size(); ++poly)
    {
        auto & current_list = m_polyList[poly].vtxList;
        const unsigned long size = current_list.size();

        if(current_list.empty())
            continue;

        for (unsigned long ix = 0; ix < size-1; ++ix) {

            segment edge_0{&current_list[ix], &current_list[(ix + 1)]};

            for (unsigned long jx = ix+1; jx < size; ++jx) {
                segment edge_1{&current_list[jx],
                               &current_list[(jx+1)%size]};

                Vertex intersection_v;
                float al1, al2;

                // see if any of the segments have a mutual endpoint
                if(!edge_1.is_bbox_overlapping_with(edge_0))
                    continue;
                if (edge_1.has_mutual_endpoint(edge_0))
                    continue;
                if(edge_1.calcIntersection(edge_0, intersection_v, al1, al2)
                   != segment::INTERSECT)
                    continue;

                auto * found_intersection = new Vertex(intersection_v);

                v_interesections.push_back(found_intersection);

                master_list.push_back(intersection(found_intersection,
                                                   al1, al2, edge_1, edge_0));

            }
        }

    }

    // finds the intersection points between every polygon edge

    vector<segment> edges, edges1;

    for (unsigned long poly1 = 0; poly1 < m_polyList.size()-1; ++poly1) {

        auto & current_poly1 = m_polyList[poly1].vtxList;
        const unsigned long size = current_poly1.size();

        edges.clear();
        for (unsigned ix = 0; ix < size; ++ix) {
            unsigned ix_next = ix+1 >= size ? 0 : ix+1;
            Vertex * current = &current_poly1[ix];
            Vertex * next = &current_poly1[ix_next];

            segment edge(current, next);
            edge.sortVertices();
            edges.push_back(edge);
        }

        sort(edges.begin(), edges.end());

        for (unsigned long poly2 = poly1+1; poly2 < m_polyList.size(); ++poly2) {

            edges1.clear();
            auto & current_poly2 = m_polyList[poly2].vtxList;
            const unsigned long size2 = current_poly2.size();

            for (unsigned ix = 0; ix < size2; ++ix)
            {
                unsigned ix_next = ix+1 >= size2 ? 0 : ix+1;
                Vertex * current = &current_poly2[ix];
                Vertex * next = &current_poly2[ix_next];

                segment edge(current, next);
                edge.sortVertices();
                edges1.push_back(edge);
            }

            sort(edges1.begin(), edges1.end());

            unsigned nEdges = edges.size();
            unsigned nEdges1 = edges1.size();

            if (nEdges < 3)
                continue;

            for (unsigned i = 0; i < nEdges; ++i) {
                auto &edge_0 = edges[i];

                for (unsigned j = 0; j < nEdges1; ++j) {

                    auto &edge_1 = edges1[j];

                    // if edge_1 is completely to the right of edge_0, then no intersection
                    // occurs, and also, since edge_1 syblings are sorted on the x-axis,
                    // we can skip it's upcoming syblings, therefore we break;
                    auto h_classify = edge_1.classify_horizontal(edge_0);
                    if (h_classify == segment::bbox_axis::end_of)
                        break;
                    else if (h_classify != segment::bbox_axis::overlaps)
                        continue;
                    else {
                        // we have horizontal overlap, let's test for vertical overlap
                        if (edge_1.classify_vertical(edge_0) != segment::bbox_axis::overlaps)
                            continue;
                    }

                    // see if any of the segments have a mutual endpoint
                    if (edge_1.has_mutual_endpoint(edge_0))
                        continue;

                    Vertex intersection_v;
                    float param1, param2;

                    // test and compute intersection
                    if (edge_0.calcIntersection(edge_1, intersection_v, param1, param2)
                        != segment::INTERSECT)
                        continue;

                    auto *found_intersection = new Vertex(intersection_v);

                    v_interesections.push_back(found_intersection);

                    master_list.push_back(intersection(found_intersection,
                                                       param1, param2,
                                                       edge_0, edge_1));

                }


            }

        }

    }

    return v_interesections;
}

// a function to fill polygon edge array(ivList) with the indices from the
// intersection master list.
// after most of the intersection master list, we use it to fill index information
// for the edge array list, and then we use it again to fill pointer data in the
// master list
void MultiPoly::fillAddress(edge_list &edges, master_intersection_list &master_list)
{
    // sort the polygons edges and master list
    sort (edges.begin(), edges.end());
    sort(master_list.begin() , master_list.end());

    // push real intersection into the polygon edges lists, for each edge push
    // it's intersecting vertex
    for (unsigned long ix =0; ix < master_list.size() ; ix++ ) {
        auto & intersection = master_list[ix];

        // todo:: break it into more cases to support point on an edge
        if ((intersection.param1 != 0 ) && (intersection.param2 != 0 )) {
            int i11 = 0, i21 = 0;
            Vertex *searchVertex1 = intersection.origin1();
            Vertex *searchVertex2 = intersection.origin2();

            // zero index is always a concrete polygon vertex ,
            // but always concrete part of the polygon, thus can be identified with
            // unique id
            while (edges[i11].vertices[0].v != searchVertex1)
                i11++;
            while (edges[i21].vertices[0].v != searchVertex2)
                i21++;

            edge_vertex Inter1(intersection.v, intersection.param1, int(ix));
            edge_vertex Inter2(intersection.v, intersection.param2, int(ix));

            edges[i11].vertices.push_back(Inter1);
            edges[i21].vertices.push_back(Inter2);
        }
        else {
            // natural polygon vertex joints can be filled with indices already
            // this are the first vertex and last vertex, that define an edge of a polygon.
            // first vertex is the start vertex, and second one is the end vertex.
            // other vertices are in between, later on we will sort them
            int vertexIndex = 0;
            Vertex *vertex = intersection.v;

            while (edges[vertexIndex].vertices[0].v != vertex )
                vertexIndex++;

            edges[vertexIndex].vertices[0].index = int(ix);

            vertexIndex = 0;
            while (edges[vertexIndex].vertices[1].v != vertex )
                vertexIndex++;

            edges[vertexIndex].vertices[1].index = int(ix);
        }

    }

    // sort ONLY the intersections of the poly edge array
//    for ( PseudoIt ps = ivList.p_list.begin(); ps != ivList.p_list.end(); ++ps)
    for (unsigned long ix = 0; ix < edges.size(); ix++) {
        auto & edge_vertices = edges[ix].vertices;
        sort(edge_vertices.data(), edge_vertices.data() + edge_vertices.size());
    }
//        sort ( ps->ilist.begin() + 1, ps->ilist.end() );

    // go over the edges and add the last vertex to each edge
    // COMPARE AS POINTERS, VERY IMPORTANT !!!!
//    PseudoIt ps;

    // setting up the indices for the intersections in the master_list,
    for (unsigned long edge = 0; edge < edges.size(); edge++) {
        // go over the intersections of each edge
        auto & edge_vertices = edges[edge].vertices;
        for (unsigned long ix = 0; ix < edge_vertices.size()-1; ++ix)
        {
            auto *first_vertex_of_edge = edge_vertices[0].v;
            int next_index = edge_vertices[ix+1].index;
            auto & intersection = master_list[edge_vertices[ix].index];

            // this is actually very simple. each edge is broken/ represented as
            // segments *--*--*-*----* etc..
            // for each point on the edge, update the master list where do we
            // go next.
            if (intersection.origin2() == first_vertex_of_edge)
                intersection.index2 = next_index;
            else// if (intersection.origin1() == first_vertex_of_edge)
                intersection.index1 = next_index;
//            else if (intersection.v == first_vertex_of_edge)
//                intersection.index1 = next_index;
        }
    }

    // setting up the self-index of each interseciton object
    for (unsigned long ix = 0 ; ix < master_list.size() ; ix++)
        master_list[ix].selfIndex = int(ix);
}

// traverse from the left most vertex of the ivList and form the queue of subsequent
// polygons done until the queue becomes empty i.e all the vertices of the
// multipolygon has been processed
// static method, depends only on it's inputm that means on the intersection master list
void MultiPoly::polygonPartition(MultiPoly &resMPoly,
                                 vector<intersection> &tempList,
                                 vector<int> &windingVector,
                                 vector<int> &directions)
{

    bool finished = false;
    Vertex *startVtx, *currVtx;
    intersection *prevInter;
    vector<intersection> &interVector = tempList; // todo:: this is a waste
    bool foundWinding = false;
    // todo:: interVector and tempList are mostly immutable so we can use indices
    // todo:: or pointers instead of objects
    stack<intersection> interStack;
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
            prevInter = &interVector[currIndex];

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
                        prevInter = &interVector[currIndex];
                        interVector[currIndex].index1 = -1;
                        currIndex = tempIndex;
                    }
                    else
                    {
                        if ( (( interVector[currIndex].origin1() == prevInter->origin1())
                              ||( interVector[currIndex].origin1() == prevInter->origin2() ))
                             && ( interVector[currIndex].index2 != -1 ) )
                        {
                            int tempIndex = interVector[currIndex].index2;
                            prevInter = &interVector[currIndex];
                            interVector[currIndex].index2 = -1;
                            currIndex = tempIndex;
                        }
                        else
                        {
                            int tempIndex = interVector[currIndex].index1;
                            prevInter = &interVector[currIndex];
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
                                win1 = xProd(*interVector[startIndex].l1.start(),
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

float MultiPoly::xProd(const Vertex &p, Vertex &q, Vertex &r) {
    return ( (q.x * ( r.y - p.y )) + (p.x*( q.y - r.y )) + (r.x * ( p.y - q.y )) );
}
