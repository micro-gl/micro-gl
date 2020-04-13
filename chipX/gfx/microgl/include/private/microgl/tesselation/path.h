#pragma once

#include <microgl/vec2.h>
#include <microgl/triangles.h>
#include <microgl/chunker.h>
#include <microgl/tesselation/curve_divider.h>
#include <microgl/tesselation/arc_divider.h>
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
            bool _created=false;

            struct tess_result {

            };

            vertex & firstPointOfCurrentSubPath() {
                auto current_path = _paths_vertices.current();
                return current_path.data[0];
            }
            vertex & lastPointOfCurrentSubPath() {
                auto current_path = _paths_vertices.current();
                return current_path.data[current_path.size-1];
            }
            int sizeOfCurrentSubPath() {
                return _paths_vertices.current().size;

            }
            vertex & firstPointOfLastSubPath() {
                auto current_path = _paths_vertices.back();
                return current_path.data[current_path.size-1];
            }
        public:

            path() = default;
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
                return *this;
            }

            auto addPath(const path & $path) -> path & {
                _paths_vertices.push_back($path);
                return *this;
            }

            auto closePath() -> path & {
                // move the pen to the first vertex of the sub-path and
                auto current_path = _paths_vertices.current();
                const vertex first_point = firstPointOfCurrentSubPath();
                const vertex last_point = lastPointOfCurrentSubPath();
                if(first_point==last_point) return;
                _paths_vertices.push_back(first_point);
//                _paths_vertices.cut_chunk();
                // start a new sub path ?
                moveTo(first_point);
                return *this;
            }

            auto lineTo(const vertex & point) -> path & {
                _paths_vertices.push_back(point);
                return *this;
            }
            auto lineTo(const number & x, const number & y) -> path & {
                return lineTo({x, y});
            }

            auto moveTo(const vertex & point) -> path & {
                _paths_vertices.cut_chunk();
                _paths_vertices.push_back(point);
                return *this;
            }
            auto moveTo(const number & x, const number & y) -> path & {
                return moveTo({x, y});
            }

            auto cubicBezierCurveTo(const vertex &cp1, const vertex &cp2, const vertex &last) -> path & {
                vertex bezier[4] = {lastPointOfCurrentSubPath(), cp1, cp2, last};
                dynamic_array<vertex> output{32};
                curve_divider<number>::compute(bezier, output,
                        CurveDivisionAlgorithm::Uniform_32, CurveType::Cubic);
                _paths_vertices.push_back(output);
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
                        CurveDivisionAlgorithm::Uniform_32, CurveType::Quadratic);
                _paths_vertices.push_back(output);
                return *this;
            }
            auto quadraticCurveTo(const number & cpx, const number & cpy,
                                  const number & lastX, const number & lastY) -> path & {
                return quadraticCurveTo({cpx, cpy}, {lastX, lastY});
            }

            auto rect(const vertex &leftTop,
                      const number &width, const number &height) -> path & {
                moveTo(leftTop);
                lineTo(leftTop.x+width, leftTop.y);
                lineTo(leftTop.x+width, leftTop.y+height);
                lineTo(leftTop.x, leftTop.y+height);
                return closePath();
            }
            auto rect(const number &x, const number &y,
                      const number &width, const number &height) -> path & {
                return rect({x, y}, width, height);
            }

            auto arc(const vertex &point, const number &radius,
                     const number &startAngle, const number &endAngle,
                     bool anti_clockwise) -> path & {
                dynamic_array<vertex> output{32};
                arc_divider<number>::compute(output, radius, point.x, point.y,
                        startAngle, endAngle, anti_clockwise);
                _paths_vertices.push_back(output);
                return *this;
            }
            auto arc(const number &x, const number &y, const number &radius,
                     const number &startAngle, const number &endAngle,
                     bool anti_clockwise) -> path & {
                return arc({x, y}, radius, startAngle, endAngle, anti_clockwise);
            }

            void build() {
                _paths_vertices.cut_chunk();
                _created=true;
            }

        };


    }
}

#include "path.tpp"