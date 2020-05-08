#pragma once

#include <microgl/triangles.h>
#include <microgl/tesselation/curve_divider.h>
#include <microgl/tesselation/arc_divider.h>
#include <microgl/tesselation/stroke_tessellation.h>
#include <microgl/tesselation/planarize_division.h>

namespace microgl {
    namespace tessellation {

#define abs(a) ((a)<0 ? -(a) : (a))
        using namespace microgl;

        template<typename number>
        class path {
        private:
            using index = unsigned int;
            using vertex = vec2<number>;
            chunker<vertex> _paths_vertices;
            bool _invalid=true;
            CurveDivisionAlgorithm _bezier_curve_divider=CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium;
            unsigned _arc_divisions_count=32;
            polygons::hints _polygon_hint=polygons::hints::COMPLEX;

            const vertex & firstPointOfCurrentSubPath() const {
                auto current_path = _paths_vertices.back();
                return current_path.data[0];
            }
            vertex & lastPointOfCurrentSubPath() {
                auto current_path = _paths_vertices.back();
                return current_path.data[current_path.size-1]; // todo: size may be 0
            }
            int sizeOfCurrentSubPath() {
                return _paths_vertices.back().size;
            }
            vertex & firstPointOfLastSubPath() {
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
                _paths_vertices=std::move($path._paths_vertices);
                return *this;
            }

            int subpathsCount() const {
                return _paths_vertices.size();
            }

            auto getSubPath(index idx) -> typename chunker<vertex>::chunk {
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

            auto addPoly(const dynamic_array<vertex> & poly) -> path & {
                _paths_vertices.cut_chunk_if_current_not_empty();
                _paths_vertices.push_back(poly);
                invalidate();
                return *this;
            }

            auto closePath() -> path & {
                if(sizeOfCurrentSubPath()==0) return *this;
                // move the pen to the first vertex of the sub-path and
                auto current_path = _paths_vertices.back();
                const vertex &first_point = firstPointOfCurrentSubPath();
                // if two last points equal the first one, it is a close path signal
                _paths_vertices.push_back(first_point);
                _paths_vertices.push_back(first_point);
                _paths_vertices.cut_chunk_if_current_not_empty();
                invalidate();
//                moveTo(first_point);
                return *this;
            }

            auto linesTo(const std::initializer_list<vertex> &list) -> path & {
                for(auto it = list.begin(); it != list.end(); ++it)
                    lineTo(*it);
                return *this;
            }
            auto lineTo(const vertex & point) -> path & {
                auto current_path = _paths_vertices.back();
                bool avoid = current_path.size && current_path.data[current_path.size-1]==point;
                if(!avoid) {
                    if(current_path.size==0) {
                        const auto paths= subpathsCount();
                        if(paths>=2) {
                            auto last_path = _paths_vertices[paths-2];
                            _paths_vertices.push_back(last_path.data[last_path.size-1]); // todo: size may be 0
                        } //else _paths_vertices.push_back({0, 0});
                    }
                    _paths_vertices.push_back(point);
                    invalidate();
                }
                return *this;
            }
            auto lineTo(const number & x, const number & y) -> path & {
                return lineTo({x, y});
            }

            auto moveTo(const vertex & point) -> path & {
                _paths_vertices.cut_chunk_if_current_not_empty();
                _paths_vertices.push_back(point);
                invalidate();
                return *this;
            }
            auto moveTo(const number & x, const number & y) -> path & {
                return moveTo({x, y});
            }

            auto cubicBezierCurveTo(const vertex &cp1, const vertex &cp2, const vertex &last) -> path & {
                vertex bezier[4] = {lastPointOfCurrentSubPath(), cp1, cp2, last};
                dynamic_array<vertex> output{32};
                curve_divider<number>::compute(bezier, output, _bezier_curve_divider, CurveType::Cubic);
                _paths_vertices.push_back(output);
                invalidate();
                return *this;
            }
            auto cubicBezierCurveTo(const number & cp1x, const number & cp1y,
                                    const number & cp2x, const number & cp2y,
                                    const number & lastX, const number & lastY) -> path & {
                return cubicBezierCurveTo({cp1x, cp1y}, {cp2x, cp2y}, {lastX, lastY});
            }

            auto quadraticCurveTo(const vertex &cp, const vertex &last) -> path & {
                vertex bezier[3] = {lastPointOfCurrentSubPath(), cp, last};
                dynamic_array<vertex> output{32};
                curve_divider<number>::compute(bezier, output,
                        _bezier_curve_divider, CurveType::Quadratic);
                _paths_vertices.push_back(output);
                invalidate();
                return *this;
            }
            auto quadraticCurveTo(const number & cpx, const number & cpy,
                                  const number & lastX, const number & lastY) -> path & {
                return quadraticCurveTo({cpx, cpy}, {lastX, lastY});
            }

            auto rect(const vertex &leftTop, const number &width,
                    const number &height, const bool cw=true) -> path & {
                moveTo(leftTop);
                if(cw) {
                    if(width) lineTo(leftTop.x+width, leftTop.y);
                    if(height) lineTo(leftTop.x+width, leftTop.y+height);
                    if(width && height) lineTo(leftTop.x, leftTop.y+height);
                } else {
                    if(height) lineTo(leftTop.x, leftTop.y+height);
                    if(width) lineTo(leftTop.x+width, leftTop.y+height);
                    if(width && height) lineTo(leftTop.x+width, leftTop.y);
                }
                return closePath();
            }
            auto rect(const number &x, const number &y,
                      const number &width, const number &height, const bool cw=true) -> path & {
                return rect({x, y}, width, height, cw);
            }

            auto arc(const vertex &point, const number &radius,
                     const number &startAngle, const number &endAngle,
                     bool anti_clockwise) -> path & {
                dynamic_array<vertex> output{32};
                arc_divider<number>::compute(output, radius, point.x, point.y,
                        startAngle, endAngle, _arc_divisions_count, anti_clockwise);
                _paths_vertices.push_back(output);
                invalidate();
                return *this;
            }
            auto arc(const number &x, const number &y, const number &radius,
                     const number &startAngle, const number &endAngle,
                     bool anti_clockwise) -> path & {
                return arc({x, y}, radius, startAngle, endAngle, anti_clockwise);
            }

            auto config(CurveDivisionAlgorithm bezier_curve_divider, unsigned arc_divisions_count,
                        polygons::hints polygon_hint) -> path & {
                _bezier_curve_divider=bezier_curve_divider;
                _arc_divisions_count=arc_divisions_count;
                _polygon_hint=polygon_hint;
                invalidate();
                return *this;
            }

            auto invalidate() -> path & {
                _invalid=true;
                return *this;
            }

            struct buffers {
                dynamic_array<vertex> DEBUG_output_trapezes;
                dynamic_array<vertex> output_vertices;
                dynamic_array<index> output_indices;
                dynamic_array<triangles::boundary_info> output_boundary;
                triangles::indices output_indices_type;
                void clear() {
                    DEBUG_output_trapezes.clear();
                    output_vertices.clear();
                    output_indices.clear();
                    output_boundary.clear();
                }
            };

            buffers & tessellateFill(const fill_rule &rule=fill_rule::non_zero,
                                     const tess_quality &quality=tess_quality::better) {
                const bool was_computed=_tess_fill.output_vertices.size()!=0;
                if(_invalid || !was_computed) {
                    _invalid=false;
                    _tess_fill.clear();
                    planarize_division<number>::compute(
                            _paths_vertices, rule, quality,
                            _tess_fill.output_vertices,
                            _tess_fill.output_indices_type,
                            _tess_fill.output_indices,
                            &_tess_fill.output_boundary,
                            &_tess_fill.DEBUG_output_trapezes);
//                            nullptr);
                }
                return _tess_fill;
            }

            buffers & tessellateStroke(const number & stroke_width=number(1),
                                       const stroke_cap &cap=stroke_cap::butt,
                                       const stroke_line_join &line_join=stroke_line_join::bevel,
                                       const int miter_limit=4) {
                const bool was_computed=_tess_stroke.output_vertices.size()!=0;
                if(_invalid || !was_computed) {
                    _invalid=false;
                    _tess_stroke.clear();
                    unsigned paths = _paths_vertices.size();
                    for (unsigned ix = 0; ix < paths; ++ix) {
                        auto chunk = _paths_vertices[ix];
                        if(chunk.size==0) continue;
                        bool isClosing = chunk.size >= 3 && chunk.data[0] == chunk.data[chunk.size - 1]
                                         && chunk.data[0] == chunk.data[chunk.size - 2];
                        stroke_tessellation<number>::compute_with_dashes(
                                stroke_width,
                                isClosing,
                                cap, line_join,
                                stroke_gravity::inward,
                                miter_limit,
                                chunk.data, chunk.size - 0,
                                _tess_stroke.output_vertices,
                                _tess_stroke.output_indices,
                                _tess_stroke.output_indices_type,
//                                nullptr);
                                &_tess_stroke.output_boundary);
                    }
                }
                return _tess_stroke;
            }

        private:
            buffers _tess_fill;
            buffers _tess_stroke;
        };

    }

}

#include "path.tpp"