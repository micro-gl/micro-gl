#pragma once

#include <microgl/triangles.h>
#include <microgl/micro_gl_traits.h>
#include <microgl/tesselation/curve_divider.h>
#include <microgl/tesselation/elliptic_arc_divider.h>
#include <microgl/tesselation/stroke_tessellation.h>
#include <microgl/tesselation/planarize_division.h>

namespace microgl {
    namespace tessellation {

        using namespace microgl;

        template<typename number, template<typename...> class container_template_type>
        class path {
        private:
            using index = unsigned int;
            using vertex = vec2<number>;
            chunker<vertex, container_template_type> _paths_vertices;
            bool _invalid=true;

            vertex firstPointOfCurrentSubPath() const {
                auto current_path = _paths_vertices.back();
                return current_path.data[0];
            }
            vertex lastPointOfCurrentSubPath() {
                auto current_path = _paths_vertices.back();
                return current_path.data[current_path.size-1]; // todo: size may be 0
            }
            int sizeOfCurrentSubPath() {
                return _paths_vertices.back().size;
            }
            vertex firstPointOfLastSubPath() {
                auto current_path = _paths_vertices.back();
                return current_path.data[current_path.size-1];
            }
        public:

            path() = default;
            path(const path & $path) {
                _paths_vertices=$path._paths_vertices;
            }

            ~path() = default;

            path &operator=(const path & $path) {
                _paths_vertices=$path._paths_vertices;
                return *this;
            }
            path &operator=(path && $path) noexcept {
                _paths_vertices=traits::move($path._paths_vertices);
                return *this;
            }

            int subpathsCount() const {
                return _paths_vertices.size();
            }

            auto getSubPath(index idx) -> typename chunker<vertex, container_template_type>::chunk {
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

            auto addPoly(const container_template_type<vertex> & poly) -> path & {
                return addPoly(poly.data(), poly.size());
            }

            auto addPoly(const vertex *poly, unsigned size) -> path & {
                if(size==0) return *this;
                _paths_vertices.cut_chunk_if_current_not_empty();
                for (unsigned ix = 0; ix < size; ++ix)
                    lineTo(poly[ix]);
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
                for (const auto & item : list)
                    lineTo(item);
                return *this;
            }

            /**
             * unpacked variadic version of lines to
             *
             */
            template<typename... Args>
            auto linesTo2(const number & x, const number & y, Args... args) -> path & {
                linesTo2(x, y);
                linesTo2(args...);
                return *this;
            }
            auto linesTo2(const number & x, const number & y) -> path & {
                lineTo({x, y});
                return *this;
            }

            auto lineTo(const vertex & point, number threshold=number(1)) -> path & {
                auto current_path = _paths_vertices.back();
                threshold=threshold<0 ? 0 : threshold;
                bool avoid=false;
                if(current_path.size) {
                    const auto vec= current_path.data[current_path.size-1]-point;
                    avoid= (vec.x*vec.x + vec.y*vec.y)<=threshold*threshold;
                }
                if(!avoid) {
                    if(current_path.size==0) {
                        const auto paths= subpathsCount();
                        if(paths>=2) {
                            auto last_path = _paths_vertices[paths-2];
                            _paths_vertices.push_back(last_path.data[last_path.size-1]);
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
                                    CurveDivisionAlgorithm bezier_curve_divider=
                                            CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small)  {
                vertex bezier[4] = {lastPointOfCurrentSubPath(), cp1, cp2, last};
                container_template_type<vertex> output{32};
                curve_divider<number, container_template_type>::compute(
                        bezier, output, bezier_curve_divider, CurveType::Cubic);
                for (unsigned ix = 0; ix < output.size(); ++ix)
                    lineTo(output[ix]);
                invalidate();
                return *this;
            }

            auto quadraticCurveTo(const vertex &cp, const vertex &last,
                    CurveDivisionAlgorithm bezier_curve_divider=
                            CurveDivisionAlgorithm::Adaptive_tolerance_distance_Small)
                    -> path & {
                vertex bezier[3] = {lastPointOfCurrentSubPath(), cp, last};
                container_template_type<vertex> output{32};
                curve_divider<number, container_template_type>::compute(
                        bezier, output, bezier_curve_divider, CurveType::Quadratic);
                for (unsigned ix = 0; ix < output.size(); ++ix)
                    lineTo(output[ix]);
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
                container_template_type<vertex> output{divisions_count};
                elliptic_arc_divider<number, container_template_type>::compute(
                        output, point.x, point.y, radius_x, radius_y,
                        rotation, startAngle, endAngle, divisions_count, anti_clockwise);
                for (int ix = 0; ix < output.size(); ++ix)
                    lineTo(output[ix]);
                return *this;
            }

            auto invalidate() -> path & {
                _invalid=true;
                return *this;
            }

            struct buffers {
                container_template_type<vertex> DEBUG_output_trapezes{};
                container_template_type<vertex> output_vertices{};
                container_template_type<index> output_indices{};
                container_template_type<triangles::boundary_info> output_boundary{};
                triangles::indices output_indices_type{};
                explicit buffers()= default;
                buffers(buffers && val) noexcept {
                    move_from(val);
                }
                buffers & operator=(buffers && val) noexcept {
                    move_from(val); return *this;
                }
                void move_from(buffers & val) {
                    DEBUG_output_trapezes=traits::move(val.DEBUG_output_trapezes);
                    output_vertices=traits::move(val.output_vertices);
                    output_indices=traits::move(val.output_indices);
                    output_boundary=traits::move(val.output_boundary);
                    output_indices_type=val.output_indices_type;
                }
                void drain() {
                    DEBUG_output_trapezes.drain();
                    output_vertices.drain();
                    output_indices.drain();
                    output_boundary.drain();
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
                    bits bits_used=0;
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
                    auto bits = used_integer_bits(max);
                    auto base = 1u<<bits;
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
                    planarize_division<number, container_template_type>::compute(
                            _paths_vertices, rule, quality,
                            _tess_fill.output_vertices,
                            _tess_fill.output_indices_type,
                            _tess_fill.output_indices,
                            compute_boundary_buffer ? &_tess_fill.output_boundary : nullptr,
                            debug_trapezes ? &_tess_fill.DEBUG_output_trapezes : nullptr);
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
                        if(chunk.size==0) continue;
                        bool isClosing = chunk.size >= 3 && chunk.data[chunk.size - 3] == chunk.data[chunk.size - 1]
                                         && chunk.data[chunk.size - 3] == chunk.data[chunk.size - 2];
                        stroke_tessellation<number>::compute_with_dashes(
                                stroke_width,
                                isClosing,
                                cap, line_join,
                                miter_limit,
                                stroke_dash_array, stroke_dash_offset,
                                chunk.data, chunk.size - (isClosing?2:0),
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

        private:
            buffers _tess_fill;
            buffers _tess_stroke;
        };

    }

}
