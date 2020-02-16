#pragma once

#include <microgl/matrix.h>
#include <microgl/math.h>
#include <microgl/vec4.h>

namespace microgl {

    template<typename number>
    class matrix_4x4 : public matrix<number, 4, 4> {
    private:
        using index = unsigned;
        using vertex3 = vec3<number>;
        using vertex4 = vec4<number>;
        using base__ =  matrix<number, 4, 4>;
        using type_ref = number &;
        using const_type_ref = const number &;
        using matrix_ref = matrix_4x4<number> &;
        using const_matrix_ref = const matrix_4x4<number> &;

        static const index SX = 0;
        static const index SY = 5;
        static const index SZ = 10;
        static const index numberX = 3;
        static const index numberY = 7;
        static const index numberZ = 11;

    public:
        // in this derived class I overload * operator, this will default in
        // c++ to hiding all previous * overloading, so we have to re-expose it
        using base__::operator*;

        static
        matrix_4x4 translate(const_type_ref tx, const_type_ref ty, const_type_ref tz) {
            matrix_4x4 mat{};
            mat[numberX] = tx;
            mat[numberY] = ty;
            mat[numberZ] = tz;
            return mat;
        }

        static
        matrix_4x4 scale(const_type_ref sx, const_type_ref sy, const_type_ref sz) {
            matrix_4x4 mat{};
            mat[SX] = sx;
            mat[SY] = sy;
            mat[SZ] = sz;
            return mat;
        }

        static
        matrix_4x4 rotation(const_type_ref angle, const vertex3 &axis) {
            matrix_4x4 mat{};
            const vertex3 ax = axis.normalize();
            const number s = microgl::math::sin(angle);
            const number c = microgl::math::cos(angle);
            const number c1 = number(1) - c;
            const number &x = ax.x;
            const number &y = ax.y;
            const number &z = ax.z;

            mat.setRow(0, {x * x * c1 + c, x * y * c1 - z * s, x * z * c1 + y * s });
            mat.setRow(1, {x * y * c1 + z * s, y * y * c1 + c, y * z * c1 - x * s });
            mat.setRow(2, {x * z * c1 - y * s, y * z * c1 + x * s, z * z * c1 + c });

            return mat;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // convert rotation angles (degree) to 4x4 matrix
        // NOnumberE: the angle is for orbit camera, so yaw angle must be reversed before
        // matrix computation.
        //
        // numberhe order of rotation is Roll->Yaw->Pitch (Rx*Ry*Rz)
        // Rx: rotation about X-axis, pitch
        // Ry: rotation about Y-axis, yaw(heading)
        // Rz: rotation about Z-axis, roll
        //    Rx           Ry          Rz
        // |1  0   0| | Cy  0 Sy| |Cz -Sz 0|   | CyCz        -CySz         Sy  |
        // |0 Cx -Sx|*|  0  1  0|*|Sz  Cz 0| = | SxSyCz+CxSz -SxSySz+CxCz -SxCy|
        // |0 Sx  Cx| |-Sy  0 Cy| | 0   0 1|   |-CxSyCz+SxSz  CxSySz+SxCz  CxCy|
        ///////////////////////////////////////////////////////////////////////////////
        static
        matrix_4x4 transform(const vertex3 & rotation = {0, 0, 0},
                            const vertex3 & translation = {0, 0, 0},
                            const vertex3 & scale = {1, 1, 1})
        {
            matrix_4x4 result {};
            number sx, sy, sz, cx, cy, cz;
            vertex3 vec;
            // rotation angle about X-axis (pitch)
            sx = microgl::math::sin(rotation.x);
            cx = microgl::math::cos(rotation.x);
            // rotation angle about Y-axis (yaw)
            sy = microgl::math::sin(rotation.y);
            cy = microgl::math::cos(rotation.y);
            // rotation angle about Z-axis (roll)
            sz = microgl::math::sin(rotation.z);
            cz = microgl::math::cos(rotation.z);

            // determine left vector
            vertex3 left     {cy * cz * scale.x, (sx * sy * cz + cx * sz) * scale.y, (-cx * sy * cz + sx * sz) * scale.z};
            vertex3 up       {-cy * sz * scale.x, (-sx * sy * sz + cx * cz) * scale.y, (cx * sy * sz + sx * cz) * scale.z};
            vertex3 forward  {sy * scale.x, -sx * cy * scale.y, cx * cy * scale.z};

            result.setColumn(0, left);
            result.setColumn(1, up);
            result.setColumn(2, forward);
            result.setColumn(3, translation);

            return result;
        }


        static
        matrix_ref fast_3x3_in_place_transpose(matrix_ref mat) {
            // very fast 3x3 block in-place transpose of a 4x4 matrix
            number tmp;
            tmp = mat[1];  mat[1] = mat[4];  mat[4] = tmp;
            tmp = mat[2];  mat[2] = mat[8];  mat[8] = tmp;
            tmp = mat[6];  mat[6] = mat[9];  mat[9] = tmp;
            return mat;
        }

        matrix_4x4() {
            identity();
        };

        matrix_4x4(const std::initializer_list<number> &list) :
                            base__(list) {}

        matrix_4x4(const_type_ref fill_value) :
                base__(fill_value) {}

        matrix_4x4(const base__ & mat) :
                base__(mat) {}

        template<typename number2>
        matrix_4x4(const matrix<number2, 4, 4> & mat) :
                base__(mat) {}

        virtual ~matrix_4x4() = default;

        void fill_diagonal(const_type_ref value) {
            index next = 0;
            for (index row = 0; row < this->rows(); ++row, next+=4)
                this->_data[next++] = value;
        }

        matrix_ref identity() {
            this->fill(0);
            number fill_one{1};
            fill_diagonal(fill_one);
            return *this;
        }

        void setColumn(const index column_index, const vertex3 & val) {
            auto & me = *this;
            me[column_index] = val.x;
            me[column_index+4] = val.y;
            me[column_index+8] = val.z;
        }

        void setRow(const index row_index, const vertex3 & val) {
            auto & me = *this;
            const index start = row_index*4;
            me[start + 0] = val.x;
            me[start + 1] = val.y;
            me[start + 2] = val.z;
        }

        vertex4 operator*(const vertex4 & point) const {
            vertex4 res;
            const auto & m = (*this);

            res.x = m[0]*point.x + m[1]*point.y + m[2]*point.z + m[3]*point.w;
            res.y = m[4]*point.x + m[5]*point.y + m[6]*point.z + m[7]*point.w;
            res.z = m[8]*point.x + m[9]*point.y + m[10]*point.z + m[11]*point.w;
            res.w = m[12]*point.x + m[13]*point.y + m[14]*point.z + m[15]*point.w;

            return res;
        }

    };

}
