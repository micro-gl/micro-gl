#pragma once

#include <microgl/triangles.h>
#include <microgl/tesselation/curve_divider.h>
#include <microgl/tesselation/elliptic_arc_divider.h>
#include <microgl/tesselation/stroke_tessellation.h>
#include <microgl/tesselation/planarize_division.h>

namespace microgl {
    namespace tessellation {

        using namespace microgl;

        template<typename number>
        class path {
        private:
            using index = unsigned int;
            using vertex = vec2<number>;
            chunker<vertex> _paths_vertices;
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
                const vertex first_point = firstPointOfCurrentSubPath();
                const vertex last_point = lastPointOfCurrentSubPath();
                // if two last points equal the first one, it is a close path signal
                _paths_vertices.push_back(last_point);
                _paths_vertices.push_back(last_point);
                _paths_vertices.cut_chunk_if_current_not_empty();
                invalidate();
                return *this;
            }

            auto linesTo(const std::initializer_list<vertex> &list) -> path & {
                for(auto it = list.begin(); it != list.end(); ++it)
                    lineTo(*it);
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

            auto cubicBezierCurveTo(const vertex &cp1, const vertex &cp2, const vertex &last,
                                    CurveDivisionAlgorithm bezier_curve_divider=
                                            CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium) -> path & {
                vertex bezier[4] = {lastPointOfCurrentSubPath(), cp1, cp2, last};
                dynamic_array<vertex> output{32};
                curve_divider<number>::compute(bezier, output, bezier_curve_divider, CurveType::Cubic);
                for (unsigned ix = 0; ix < output.size(); ++ix)
                    lineTo(output[ix]);
                invalidate();
                return *this;
            }

            auto quadraticCurveTo(const vertex &cp, const vertex &last,
                    CurveDivisionAlgorithm bezier_curve_divider=
                            CurveDivisionAlgorithm::Adaptive_tolerance_distance_Medium)
                    -> path & {
                vertex bezier[3] = {lastPointOfCurrentSubPath(), cp, last};
                dynamic_array<vertex> output{32};
                curve_divider<number>::compute(bezier, output, bezier_curve_divider, CurveType::Quadratic);
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
                dynamic_array<vertex> output{divisions_count};
                elliptic_arc_divider<number>::compute(output, point.x, point.y, radius_x, radius_y,
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
                dynamic_array<vertex> DEBUG_output_trapezes;
                dynamic_array<vertex> output_vertices;
                dynamic_array<index> output_indices;
                dynamic_array<triangles::boundary_info> output_boundary;
                triangles::indices output_indices_type;
                explicit buffers()= default;
                buffers(buffers && val) noexcept {
                    move(val); output_indices_type=triangles::indices::TRIANGLES_STRIP;
                }
                buffers &operator=(buffers && val) noexcept {
                    move(val); return *this;
                }
                buffers &move(buffers && val) {
                    DEBUG_output_trapezes=std::move(val.DEBUG_output_trapezes);
                    output_vertices=std::move(val.output_vertices);
                    output_indices=std::move(val.output_indices);
                    output_boundary=std::move(val.output_boundary);
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
                number stroke_width;stroke_cap cap;
                stroke_line_join line_join; int miter_limit;
                std::initializer_list<int> stroke_dash_array;
                int stroke_dash_offset;
                bool operator==(const stroke_cache_info &val) {
                    bool a= stroke_width==val.stroke_width &&
                            cap==val.cap &&
                            line_join==val.line_join &&
                            miter_limit==val.miter_limit &&
                            stroke_dash_offset==val.stroke_dash_offset;
                    bool b=stroke_dash_array.size()==val.stroke_dash_array.size();
                    if(b) {
                        auto *iter_a=stroke_dash_array.begin();
                        auto *iter_b=val.stroke_dash_array.begin();
                        while (iter_a!=stroke_dash_array.end()) {
                            if(*iter_a!=*iter_b) {
                                b=false; break;
                            }
                            iter_a++; iter_b++;
                        }
                    }
                    return a && b;
                }
            };

            stroke_cache_info _latest_stroke_cache_info;
            fill_cache_info _latest_fill_cache_info;

        public:
            buffers & tessellateFill(const fill_rule &rule=fill_rule::non_zero,
                                     const tess_quality &quality=tess_quality::better) {
                fill_cache_info info{rule, quality};
                const bool was_computed=(info==_latest_fill_cache_info) &&
                        _tess_fill.output_vertices.size()!=0;
                if(_invalid || !was_computed) {
                    _latest_fill_cache_info=info;
                    _invalid=false;
                    _tess_fill.clear();
                    planarize_division<number>::compute(
                            _paths_vertices, rule, quality,
                            _tess_fill.output_vertices,
                            _tess_fill.output_indices_type,
                            _tess_fill.output_indices,
                            &_tess_fill.output_boundary,
                            &_tess_fill.DEBUG_output_trapezes);
//                            nullptr,
//                            nullptr);
                }
                return _tess_fill;
            }

            buffers & tessellateStroke(const number & stroke_width=number(1),
                                       const stroke_cap &cap=stroke_cap::butt,
                                       const stroke_line_join &line_join=stroke_line_join::bevel,
                                       const int miter_limit=4,
                                       const std::initializer_list<int> & stroke_dash_array={},
                                       int stroke_dash_offset=0) {
                stroke_cache_info info{stroke_width, cap, line_join, miter_limit,
                                       stroke_dash_array, stroke_dash_offset};
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
//                                nullptr);
                                &_tess_stroke.output_boundary);
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

#include "path.tpp"