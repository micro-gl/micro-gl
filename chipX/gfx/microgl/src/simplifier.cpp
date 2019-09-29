#include <microgl/tesselation/simplifier.h>

namespace tessellation {

    struct node {
        // may have missing null children, that were removed
        std::vector<node *> children;
        int index_poly = -1;
    };

    // Use the sign of the determinant of vectors (AB,AM), where M(X,Y) is the query point:
    // position = sign((Bx - Ax) * (Y - Ay) - (By - Ay) * (X - Ax))

    //    Input:  three points p, a, b
    //    Return: >0 for P2 left of the line through a and b
    //            =0 for P2  on the line
    //            <0 for P2  right of the line
    //    See: Algorithm 1 "Area of Triangles and Polygons"
    inline int
    classify_point(const vertex & point, const vertex &a, const vertex & b)
    {
        return int((b.x - a.x) * (point.y - a.y)
                   - (point.x - a.x) * (b.y - a.y) );
    }

    inline int
    isLeft(const vertex & point, const vertex &a, const vertex & b)
    {
        return classify_point(point, a, b) > 0;
    }

    int
    point_inside_simple_polygon_wn(const vertex &point,
                                   const vertex *poly,
                                   const int size)
    {
        // the  winding number counter
        int    wn = 0;

        // loop through all edges of the polygon
        // edge from V[i] to  V[i+1]
        for (int ix=0; ix < size; ix++) {
            // start y <= P.y
            if (poly[ix].y < point.y) {
                // an upward crossing
                if (poly[ix + 1].y >= point.y)
                    // P left of  edge
                    if (isLeft(point, poly[ix], poly[ix + 1]) > 0)
                        ++wn;
            }
            else {                        // start y > P.y (no test needed)
                // a downward crossing
                if (poly[ix + 1].y < point.y)
                    // P right of  edge
                    if (isLeft(point, poly[ix], poly[ix + 1]) < 0)
                        --wn;
            }
        }

        return wn!=0;
    }

    bool point_inside_simple_polygon_cn(const vertex &point,
                                        const vertex *poly,
                                        const int size) {

        int cn = 0;

        // loop through all edges of the polygon
        for (int ix=0; ix < size; ix++) {
            // an upward crossing
//            if ((poly[ix].y<=point.y && poly[ix + 1].y>point.y)
                // a downward crossing
//                || (poly[ix].y>point.y && poly[ix + 1].y<=point.y)) {
            if ((poly[ix].y<point.y && poly[ix + 1].y>=point.y)
                // a downward crossing
                || (poly[ix].y>=point.y && poly[ix + 1].y<point.y)) {
                // compute  the actual edge-ray intersect x-coordinate
                float vt = (point.y  - poly[ix].y) /
                            (poly[ix + 1].y - poly[ix].y);

                // P.x < intersect
                if (point.x < poly[ix].x + vt * (poly[ix + 1].x - poly[ix].x))
                    // a valid crossing of y=P.y right of P.x
                    ++cn;
            }
        }

        // 0 if even (out), and 1 if  odd (in)
        return (cn&1);
    }

    // tests if a point is completely inside, excluding boundary
    bool point_inside_convex_poly_interior(const vertex &point,
                                           const vertex * poly,
                                           int size,
                                           bool CCW = true) {
        bool fails = true;
        int direction = CCW ? 1 : -1;

        for (int ix = 0; ix < size; ++ix) {
            int a = ix;
            int b = (ix+1)==size ? 0 : ix+1;

            // handle a common degenerate case
            if(poly[a]==poly[b])
                continue;

            int classify = classify_point(point, poly[a], poly[b]);

            // we also fail on boundary
            fails = classify * direction >= 0;

            if(fails)
                return false;
        }

        return true;
    }

    // the extremal left-bottom most vertex is always a convex vertex
    int find_convex_vertex(vertex * poly,
                           const int size) {
        int index = 0;
        vertex value = poly[0];

        for (int ix = 0; ix < size; ++ix) {
            auto & v = poly[ix];

            if(v.x < value.x) {
                value = v;
                index = ix;
            }
            else if(v.x==value.x) {
                if(v.y > value.y) {
                    value = v;
                    index = ix;
                }
            }

        }

        return index;
    }

    // find a point via the diagonal method, a linear time algorithm
    vertex find_point_in_simple_polygon_interior(vertex * poly,
                                        const int size,
                                        bool CCW = true) {
        int vi = find_convex_vertex(poly, size);
        int ai = vi-1 < 0 ? size-1 : vi-1;
        int bi = vi+1 == size ? 0 : vi+1;

        vertex v = poly[vi];
        vertex a = poly[ai];
        vertex b = poly[bi];
        vertex triangle[3] = {a, v, b};

        vertex q;
        auto min_qv_distance = poly[0].x;
        min_qv_distance = 0;

        for (int ix = 0; ix < size; ++ix) {
            const auto & q_candidate = poly[ix];

            // avoid degenerate case
            if(v==q)
                continue;

            if(point_inside_convex_poly_interior(q, triangle, 3, CCW)) {
                auto qv_distance = (q.x-v.x)*(q.x-v.x) +
                        (q.y-v.y)*(q.x-v.y);
                if(qv_distance < min_qv_distance) {
                    q = q_candidate;
                    min_qv_distance = qv_distance;
                }

            }

        }

        if(min_qv_distance==0)
            return (a+b)/2;

        return (q+v)/2;
    }

    /**
     * since polygons are non-intersecting, then it is enough to test one point inclusion.
     * if one point is inside a polygon, then the entire polygon is inside, otherwise
     * it will contradict the fact that they are non-intersecting.
     * polygons may touch one another. this is a nice and fast algorithm
     *
     * 1=poly 1 inside poly 2
     * -1=poly 2 inside poly 1
     * 0=poly 1 and poly 2 are disjoint/separable
     */
    int compare_simple_non_intersecting_polygons(vertex * poly_1, int size_1, bool poly_1_CCW,
                                                 vertex * poly_2, int size_2, bool poly_2_CCW) {

        vertex sample = find_point_in_simple_polygon_interior(poly_1, size_1, poly_1_CCW);

        bool poly_1_inside_2 = point_inside_simple_polygon_wn(sample, poly_2, size_2);

        if(poly_1_inside_2)
            return 1;

        sample = find_point_in_simple_polygon_interior(poly_2, size_2, poly_2_CCW);
        bool poly_2_inside_1 = point_inside_simple_polygon_wn(sample, poly_1, size_1);

        if(poly_2_inside_1)
            return -1;

        return 0;
    }

    void compute_component_tree_recurse(node * root,
                                        node * current,
                                        chunker<vec2_f> & components,
                                        const vector<direction> &directions) {
        int root_children_count = root->children.size();
        int compare;

        auto poly_current = components[current->index_poly];

        if(root->index_poly != -1) {
            auto poly_root = components[root->index_poly];

            compare = compare_simple_non_intersecting_polygons(
                    poly_current.data,
                    poly_current.size - 1,
                    directions[current->index_poly]==direction::CCW,
                    poly_root.data,
                    poly_root.size-1,
                    directions[root->index_poly]==direction::CCW);

            // compare against the root polygon

            switch (compare) {
                case 1: // current inside the root node
                    // we need to find a suitable place for it among children later
                    break;
                case -1: // root inside current node
                    // bubble root down to current and exit
                    compute_component_tree_recurse(current, root, components, directions);
                    return;
//                    throw std::runtime_error("weird !!!");
                    break;
                case 0: // complete strangers, let's exit
                    return;
            }

        }

        // lets' go over the root's children
        for (int ix = 0; ix < root_children_count; ++ix) {
            auto * child_node = root->children[ix];

            if(child_node== nullptr)
                continue;

            int child_poly_index = child_node->index_poly;
            auto child_poly = components[child_poly_index];

            compare = compare_simple_non_intersecting_polygons(
                    poly_current.data,
                    poly_current.size-1,
                    directions[current->index_poly]==direction::CCW,
                    child_poly.data,
                    child_poly.size-1,
                    directions[child_poly_index]==direction::CCW);

            switch (compare) {
                case 1: // current inside the child, bubble it down, and exit
                    compute_component_tree_recurse(child_node, current, components, directions);
                    return;
                case -1: // child inside current node, so add it to the current node
                    // and also, disconnect the child from the root, so it will have one parent
                    // this is why I need a linked list
                    current->children.push_back(child_node);
                    root->children[ix] = nullptr;
                    break;
                case 0: // complete strangers
                    break;
            }

        }

        // if we made it here, it means current node is direct child of this root
        root->children.push_back(current);
    }

    void compute_component_tree(chunker<vec2_f> & components,
                                const vector<direction> &directions) {

        node nodes[components.size()];
        node root{};

        for (unsigned long ix = 0; ix < components.size(); ++ix)
        {
            auto * current = &(nodes[ix]);
            current->index_poly = int(ix);

            // now start hustling
            compute_component_tree_recurse(
                    &root,
                    current,
                    components,
                    directions);

        }

        int a = 0;
    }

    void simplifier::compute(chunker<vec2_f> & pieces,
                             chunker<vec2_f> & result,
                             vector<int> &directions_comps) {

        vector<direction> directions;

        simplify_components::compute(
                pieces,
                result,
                directions);

        // experiment
        compute_component_tree(result, directions);

        int a =0;
    }

}