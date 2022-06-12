/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. If this file is to be used or derived and any of the following conditions apply:
    a. for commercial usage by any company, entity or person, or
    b. by a registered company/entity/corporation for commercial or non-commercial usage,
    Then, a special permission has to be granted by the Author (Tomer Shalev) for that entity.
    As a consequence of the above conditions, ONLY entities which are not companies such as a person,
    or a group who want to use or derive this file for non-commercial usage are permitted without
    being given a special permission by the Author (Tomer Shalev)

 2. In any case of granted permission to use this file according to case 1 (see above), derivative
    work and usage of this file should be credited to the project and the author of this project.

 3. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "vec2.h"
#include "triangles.h"
#include "curve_divider.h"
#include "elliptic_arc_divider.h"
#include "stroke_tessellation.h"
#include "planarize_division.h"
#include "chunker.h"
#include "std_rebind_allocator.h"
#include "traits.h"

namespace microtess {

    /**
     * Path is a modern interface for vector graphics, where you can
     * define multiple paths with tools such as:
     * 1. Lines, Rectangles
     * 2. Bezier curves
     * 3. Elliptic arcs
     * and then you can tessellate then using:
     * 1. Fill Tessellation
     * 2. Stroke Tessellation
     *
     * Note:
     * - After Fill or Stroke Tessellation, internal buffers are cached, so re-tessellation
     *   will happen only if something is invalid. This is done to save energy.
     * - Internal cache buffers can be drained using the drainBuffers() method
     *
     * @tparam number the number type of a vertex
     * @tparam container_template_type a template of a linear container of the
     *          form Container<value_type, allocator_type> for internal usage
     * @tparam Allocator an allocator type for internal usage
     */
    template<typename number,
             template<typename...> class container_template_type=dynamic_array,
             class Allocator=std_rebind_allocator<>>
    class path {
    public:
        struct buffers;
        using index = unsigned int;
        using number_type = number;
        using vertex = microtess::vec2<number>;
        using allocator_type = Allocator;
        using value_type = vertex;
        using chunker_t = allocator_aware_chunker<vertex, container_template_type, allocator_type>;

    private:
        allocator_type _allocator;
        chunker_t _paths_vertices;
        bool _invalid=true;
        buffers _tess_fill;
        buffers _tess_stroke;

        vertex firstPointOfCurrentSubPath() const {
            auto current_path = _paths_vertices.back();
            return current_path[0];
        }
        vertex lastPointOfCurrentSubPath() {
            auto current_path = _paths_vertices.back();
            return current_path[current_path.size()-1]; // todo: size may be 0
        }
        int sizeOfCurrentSubPath() {
            return _paths_vertices.back().size();
        }
        vertex firstPointOfLastSubPath() {
            auto current_path = _paths_vertices.back();
            return current_path[current_path.size()-1];
        }

    public:
        explicit path(const allocator_type & allocator=allocator_type()) :
                    _allocator(allocator), _paths_vertices(allocator),
                    _tess_fill(allocator), _tess_stroke(allocator) {}
        path(const path & $path) : _allocator($path.get_allocator()),
                                   _paths_vertices($path._paths_vertices, _allocator),
                                   _tess_fill(_allocator), _tess_stroke(_allocator) {}
        path(path && $path) noexcept : _allocator($path.get_allocator()),
                                   _paths_vertices(microtess::traits::move($path._paths_vertices)),
                                   _tess_fill(microtess::traits::move($path._tess_fill)),
                                   _tess_stroke(microtess::traits::move($path._tess_stroke)) {}
        ~path() = default;

        path &operator=(const path & $path) {
            _paths_vertices=$path._paths_vertices;
            _tess_fill=$path._tess_fill;
            _tess_stroke=$path._tess_stroke;
            return *this;
        }
        path &operator=(path && $path) noexcept {
            _paths_vertices=microtess::traits::move($path._paths_vertices);
            _tess_fill=microtess::traits::move($path._tess_fill);
            _tess_stroke=microtess::traits::move($path._tess_stroke);
            return *this;
        }

        allocator_type get_allocator() { return _allocator; }
        int subpathsCount() const { return _paths_vertices.size(); }
        auto getSubPath(index idx) -> typename chunker_t::chunk {
            return _paths_vertices[idx];
        }
        auto clear() -> path & {
            _paths_vertices.clear();
            _tess_fill.clear();
            _tess_stroke.clear();
            invalidate();
            return *this;
        }

        auto addPath(const path & $path) -> path & {
            _paths_vertices.push_back($path._paths_vertices);
            invalidate();
            return *this;
        }

        template<class Iterable>
        auto addPoly(const Iterable & poly) -> path & {
            if(poly.size()==0) return *this;
            _paths_vertices.cut_chunk_if_current_not_empty();
            for (const auto & v : poly) lineTo(v);
            invalidate();
            return *this;
        }

        auto addPoly(const vertex *poly, unsigned size) -> path & {
            if(size==0) return *this;
            _paths_vertices.cut_chunk_if_current_not_empty();
            for (unsigned ix = 0; ix < size; ++ix) lineTo(poly[ix]);
            invalidate();
            return *this;
        }

        auto closePath() -> path & {
            if(sizeOfCurrentSubPath()==0) return *this;
            // move the pen to the first vertex of the sub-path and
            const vertex last_point = lastPointOfCurrentSubPath();
            // if two last points equal the first one, it is a close path signal
            _paths_vertices.push_back(last_point);
            _paths_vertices.push_back(last_point);
            _paths_vertices.cut_chunk_if_current_not_empty();
            invalidate();
            return *this;
        }

        /**
         * insert lines from an iterable container
         * @tparam Iterable
         * @param list
         * @return
         */
        template<class Iterable>
        auto linesTo(const Iterable & list) -> path & {
            for (const auto & item : list) lineTo(item);
            return *this;
        }

        /**
         * unpacked variadic version of lines to
         *
         */
        template<typename... Args>
        auto linesTo2(const number & x, const number & y, Args... args) -> path & {
            linesTo2(x, y); linesTo2(args...); return *this;
        }
        auto linesTo2(const number & x, const number & y) -> path & {
            lineTo({x, y}); return *this;
        }

        /**
         * unpacked variadic version of lines to
         *
         */
        template<typename... Args>
        auto linesTo3(const vec2<number> & p, Args... args) -> path & {
            linesTo3(p); linesTo3(args...); return *this;
        }
        auto linesTo3(const vec2<number> & p) -> path & {
            lineTo(p); return *this;
        }

        auto lineTo(const vertex & point, number threshold=number(1)) -> path & {
            auto current_path = _paths_vertices.back();
            threshold=threshold<0 ? 0 : threshold;
            bool avoid=false;
            if(current_path.size()) {
                const auto vec= current_path[current_path.size()-1]-point;
                avoid=(vec.x*vec.x + vec.y*vec.y)<=threshold*threshold;
            }
            if(!avoid) {
                if(current_path.size()==0) {
                    const auto paths= subpathsCount();
                    if(paths>=2) {
                        auto last_path = _paths_vertices[paths-2];
                        _paths_vertices.push_back(last_path[last_path.size()-1]);
                    }
                }
                _paths_vertices.push_back(point);
                invalidate();
            }
            return *this;
        }

        auto moveTo(const vertex & point) -> path & {
            _paths_vertices.cut_chunk_if_current_not_empty();
            _paths_vertices.push_back(point);
            invalidate();
            return *this;
        }

        path & cubicBezierCurveTo(const vertex &cp1, const vertex &cp2, const vertex &last,
                                  microtess::CurveDivisionAlgorithm bezier_curve_divider=
                                        CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small)  {
            vertex bezier[4] = {lastPointOfCurrentSubPath(), cp1, cp2, last};
            using rebind_alloc = typename allocator_type::template rebind<vertex>::other;
            container_template_type<vertex, rebind_alloc> output{32, vertex(), rebind_alloc(_allocator)};
            output.clear();
            curve_divider<number, decltype(output)>::compute(
                    bezier, output, bezier_curve_divider, CurveType::Cubic);
            for (unsigned ix = 0; ix < output.size(); ++ix) lineTo(output[ix]);
            invalidate();
            return *this;
        }

        auto quadraticCurveTo(const vertex &cp, const vertex &last,
                CurveDivisionAlgorithm bezier_curve_divider=
                        CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small)
                -> path & {
            vertex bezier[3] = {lastPointOfCurrentSubPath(), cp, last};
            using rebind_alloc = typename allocator_type::template rebind<vertex>::other;
            container_template_type<vertex, rebind_alloc> output{32, vertex(), rebind_alloc(_allocator)};
            output.clear();
            curve_divider<number, decltype(output)>::compute(
                    bezier, output, bezier_curve_divider, CurveType::Quadratic);
            for (unsigned ix = 0; ix < output.size(); ++ix) lineTo(output[ix]);
            invalidate();
            return *this;
        }

        auto rect(const number &left, const number &top, const number &width,
                const number &height, const bool cw=true) -> path & {
            moveTo({left, top});
            if(cw) {
                if(width) lineTo({left+width, top});
                if(height) lineTo({left+width, top+height});
                if(width && height) lineTo({left, top+height});
            } else {
                if(height) lineTo({left, top+height});
                if(width) lineTo({left+width, top+height});
                if(width && height) lineTo({left+width, top});
            }
            return closePath();
        }

        auto arc(const vertex &point, const number &radius,
                 const number &startAngle, const number &endAngle,
                 bool anti_clockwise, unsigned divisions_count=32) -> path & {
            return ellipse(point, radius, radius, 0, startAngle, endAngle,
                    anti_clockwise, divisions_count);
        }

        auto ellipse(const vertex &point, const number &radius_x, const number &radius_y,
                 const number & rotation, const number &startAngle, const number &endAngle,
                 bool anti_clockwise, unsigned divisions_count=32) -> path & {
            using rebind_alloc = typename allocator_type::template rebind<vertex>::other;
            container_template_type<vertex, rebind_alloc> output{divisions_count, vertex(), rebind_alloc(_allocator)};
            output.clear();
            elliptic_arc_divider<number, decltype(output)>::compute(
                    output, point.x, point.y, radius_x, radius_y,
                    rotation, startAngle, endAngle, divisions_count, anti_clockwise);
            for (int ix = 0; ix < output.size(); ++ix) lineTo(output[ix]);
            return *this;
        }

        auto invalidate() -> path & {
            _invalid=true;
            return *this;
        }

        struct buffers {
            using allocator_type_vertices = typename allocator_type::template rebind<vertex>::other;
            using allocator_type_indices = typename allocator_type::template rebind<index>::other;
            using allocator_type_boundaries = typename allocator_type::template rebind<triangles::boundary_info>::other;

            using vertices = container_template_type<vertex, allocator_type_vertices>;
            using indices = container_template_type<index, allocator_type_indices>;
            using boundaries = container_template_type<triangles::boundary_info,
                    allocator_type_boundaries>;
            using trapezes = vertices;

            vertices output_vertices;
            indices output_indices;
            boundaries output_boundary;
            trapezes DEBUG_output_trapezes;
            triangles::indices output_indices_type;
            const allocator_type & allocator;

            explicit buffers(const allocator_type & allocator) :
                    allocator(allocator),
                    output_vertices(allocator_type_vertices(allocator)),
                    output_indices(allocator_type_indices(allocator)),
                    output_boundary(allocator_type_boundaries(allocator)),
                    DEBUG_output_trapezes(allocator_type_vertices(allocator)),
                    output_indices_type() {}
            buffers(buffers && val) noexcept :
                    allocator(val.allocator),
                    output_vertices(microtess::traits::move(val.output_vertices)),
                    output_indices(microtess::traits::move(val.output_indices)),
                    output_boundary(microtess::traits::move(val.output_boundary)),
                    DEBUG_output_trapezes(microtess::traits::move(val.DEBUG_output_trapezes)),
                    output_indices_type(val.output_indices_type) {
            }
            buffers(const buffers & val) = delete;
            buffers & operator=(buffers && val) noexcept {
                move_from(val); return *this;
            }
            buffers & operator=(const buffers & val) = delete;
            void move_from(buffers & val) {
                output_vertices=microtess::traits::move(val.output_vertices);
                output_indices=microtess::traits::move(val.output_indices);
                output_boundary=microtess::traits::move(val.output_boundary);
                DEBUG_output_trapezes=microtess::traits::move(val.DEBUG_output_trapezes);
                output_indices_type=val.output_indices_type;
            }
            allocator_type get_allocator() { return allocator; }
            void drain() {
                DEBUG_output_trapezes = trapezes(allocator);
                output_vertices = vertices(allocator);
                output_indices = indices(allocator);
                output_boundary = boundaries(allocator);
            }
            void clear() {
                DEBUG_output_trapezes.clear();
                output_vertices.clear();
                output_indices.clear();
                output_boundary.clear();
            }
        };

    private:
        struct fill_cache_info {
            fill_rule rule; tess_quality quality;
            bool operator==(const fill_cache_info &val) {
                bool a= rule==val.rule &&
                        quality==val.quality;
                return a;
            }
        };

        struct stroke_cache_info {
            number stroke_width; stroke_cap cap;
            stroke_line_join line_join; int miter_limit;
            unsigned int stroke_dash_array_signature;
            int stroke_dash_offset;

            static unsigned used_integer_bits(const unsigned & value) {
                unsigned bits_used=0;
                while (value>int(1)<<(bits_used++)) {};
                return bits_used;
            }

            static unsigned int pow(unsigned base, unsigned exponent) {
                unsigned int result= 1;
                for (int ix = 0; ix < exponent; ++ix)
                    result = result * base;
                return result;
            }

            template<class Iterable>
            static unsigned int integer_sequence_to_integer(const Iterable & iterable) {
                using integer = unsigned int;
                integer max = (1u<<16) - 1;
                for (const auto & item : iterable) {
                    if(integer(item) > max)
                        max = integer(item);
                }
                const auto bits = used_integer_bits(max);
                const auto base = 1u<<bits;
                integer result=0, index=0;
                for (const auto & item : iterable)
                    result += (integer(item)) * pow(base, index++);
                return result;
            }

            bool operator==(const stroke_cache_info &val) {
                return stroke_width==val.stroke_width &&
                        cap==val.cap &&
                        line_join==val.line_join &&
                        miter_limit==val.miter_limit &&
                        stroke_dash_offset==val.stroke_dash_offset &&
                        stroke_dash_array_signature==val.stroke_dash_array_signature;
            }
        };

        stroke_cache_info _latest_stroke_cache_info;
        fill_cache_info _latest_fill_cache_info;

    public:
        template <bool APPLY_MERGE=true, unsigned MAX_ITERATIONS=200>
        buffers & tessellateFill(const fill_rule &rule=fill_rule::non_zero,
                                 const tess_quality &quality=tess_quality::better,
                                 bool compute_boundary_buffer = true,
                                 bool debug_trapezes = false) {
            fill_cache_info info{rule, quality};
            const bool was_computed=(info==_latest_fill_cache_info) &&
                    _tess_fill.output_vertices.size()!=0;
            if(_invalid || !was_computed) {
                _latest_fill_cache_info=info;
                _invalid=false;
                _tess_fill.clear();

                using planarize_division_tess = planarize_division<number,
                    decltype(_tess_fill.output_vertices),
                    decltype(_tess_fill.output_indices),
                    decltype(_tess_fill.output_boundary),
                    allocator_type,
                    APPLY_MERGE, MAX_ITERATIONS>;

                planarize_division_tess::template compute<decltype(_paths_vertices)>(
                        _paths_vertices, rule, quality,
                        _tess_fill.output_vertices,
                        _tess_fill.output_indices_type,
                        _tess_fill.output_indices,
                        compute_boundary_buffer ? &_tess_fill.output_boundary : nullptr,
                        debug_trapezes ? &_tess_fill.DEBUG_output_trapezes : nullptr,
                        _allocator);
            }
            return _tess_fill;
        }

        template<class Iterable>
        buffers & tessellateStroke(const number & stroke_width=number(1),
                                   const stroke_cap &cap=stroke_cap::butt,
                                   const stroke_line_join &line_join=stroke_line_join::bevel,
                                   const int miter_limit=4,
                                   const Iterable & stroke_dash_array={},
                                   int stroke_dash_offset=0,
                                   bool compute_boundary_buffer=true) {
            stroke_cache_info info{stroke_width, cap, line_join, miter_limit,
                                   stroke_cache_info::template integer_sequence_to_integer<Iterable>(stroke_dash_array),
                                   stroke_dash_offset};

            const bool was_computed=(info==_latest_stroke_cache_info) &&
                    _tess_stroke.output_vertices.size()!=0;
            if(_invalid || !was_computed) {
                _invalid=false;
                _latest_stroke_cache_info=info;
                _tess_stroke.clear();
                unsigned paths = _paths_vertices.size();
                for (unsigned ix = 0; ix < paths; ++ix) {
                    auto chunk = _paths_vertices[ix];
                    const auto chunk_size = chunk.size();
                    if(chunk_size==0) continue;
                    bool isClosing = chunk_size >= 3 && chunk[chunk_size - 3] == chunk[chunk_size - 1]
                                     && chunk[chunk_size - 3] == chunk[chunk_size - 2];
                    using stroke_tess = stroke_tessellation<number, decltype(_tess_stroke.output_vertices),
                                decltype(_tess_stroke.output_indices), decltype(_tess_stroke.output_boundary)>;

                    stroke_tess::template compute_with_dashes<Iterable>(
                            stroke_width,
                            isClosing,
                            cap, line_join,
                            miter_limit,
                            stroke_dash_array, stroke_dash_offset,
                            chunk.data(), chunk_size - (isClosing?2:0),
                            _tess_stroke.output_vertices,
                            _tess_stroke.output_indices,
                            _tess_stroke.output_indices_type,
                            compute_boundary_buffer ? &_tess_stroke.output_boundary: nullptr);
                }
            }
            return _tess_stroke;
        }

        void drainBuffers() {
            _paths_vertices.drain();
            _tess_fill.drain();
            _tess_stroke.drain();
            _invalid=true;
        }
        buffers & buffers_fill() { return _tess_fill; }
        buffers & buffers_stroke() { return _tess_stroke; }
        chunker_t & paths_vertices() { return _paths_vertices; }

    };
}