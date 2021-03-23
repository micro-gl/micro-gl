#pragma once

#include <microgl/matrix.h>
#include <microgl/math.h>
#include <microgl/vec2.h>

namespace microgl {

    template<typename number>
    class matrix_3x3 : public matrix<number, 3, 3> {
    private:
        using index = unsigned;
        using base__ =  matrix<number, 3, 3>;
        using type_ref = number &;
        using const_type_ref = const number &;
        using matrix_ref = matrix_3x3<number> &;
        using const_matrix_ref = const matrix_3x3<number> &;

        static const index SX = 0;
        static const index SY = 4;
        static const index TX = 2;
        static const index TY = 5;
        static const index SKEWX = 1;
        static const index SKEWY = 3;

    public:
        // in this derived class I overload * operator, this will default in
        // c++ to hiding all previous * overloading, so we have to re-expose it
        using base__::operator*;

        static
        matrix_3x3 identity() {
            return matrix_3x3{};
        }

        static
        matrix_3x3 translate(const_type_ref tx, const_type_ref ty) {
            matrix_3x3 mat{};
            mat[TX] = tx;
            mat[TY] = ty;
            return mat;
        }

        static
        matrix_3x3 scale(const_type_ref sx, const_type_ref sy) {
            matrix_3x3 mat{};
            mat[SX] = sx;
            mat[SY] = sy;
            return mat;
        }

        static
        matrix_3x3 reflect(bool x_axis, bool y_axis) {
            return scale(y_axis ? -1 : 1, x_axis ? -1 : 1);
        }

        static
        matrix_3x3 shear_x(const_type_ref angles) {
            matrix_3x3 mat{};
            mat[SKEWX] = microgl::math::tan(angles);
            return mat;
        }

        static
        matrix_3x3 shear_y(const_type_ref angles) {
            matrix_3x3 mat{};
            mat[SKEWY] = microgl::math::tan(angles);
            return mat;
        }

        static
        matrix_3x3 rotation(const_type_ref angle) {
            matrix_3x3 mat{};

            const_type_ref cos_ = microgl::math::cos(angle);
            const_type_ref sin_ = microgl::math::sin(angle);

            mat[0] = cos_;
            mat[1] = -sin_;
            mat[3] = sin_;
            mat[4] = cos_;
            return mat;
        }

        static
        matrix_3x3 rotation(const_type_ref angle,
                            const_type_ref px,
                            const_type_ref py) {
            matrix_3x3 mat{};

            const_type_ref cos_ = microgl::math::cos(angle);
            const_type_ref sin_ = microgl::math::sin(angle);

            mat[0] = cos_;
            mat[1] = -sin_;
            mat[2] = -cos_*px + sin_*py + px;

            mat[3] = sin_;
            mat[4] = cos_;
            mat[5] = -sin_*px - cos_*py + py;

            mat[6] = 0;
            mat[7] = 0;
            mat[8] = number(1);

            return mat;
        }

        static
        matrix_3x3 rotation(const_type_ref angle,
                            const_type_ref px,
                            const_type_ref py,
                            const_type_ref sx,
                            const_type_ref sy) {
            matrix_3x3 mat{};

            const_type_ref cos_ = microgl::math::cos(angle);
            const_type_ref sin_ = microgl::math::sin(angle);

            mat[0] = sx*cos_;
            mat[1] = -sy*sin_;
            mat[2] = -sx*cos_*px + sy*sin_*py + px;

            mat[3] = sx*sin_;
            mat[4] = sy*cos_;
            mat[5] = -sx*sin_*px - sy*cos_*py + py;

            mat[6] = 0;
            mat[7] = 0;
            mat[8] = number(1);

            return mat;
        }

        matrix_3x3() {
            identity_fill();
        };

        template<class Iterable>
        matrix_3x3(const Iterable & list) : base__{list} {
        }

        matrix_3x3(const_type_ref fill_value) :
                base__(fill_value) {}

        matrix_3x3(const base__ & mat) :
                base__(mat) {}

        template<typename T2>
        matrix_3x3(const matrix<T2, 3, 3> & mat) :
                base__(mat) {}

        virtual ~matrix_3x3() = default;

        vec2<number> operator*(const vec2<number>& point) const {
            vec2<number> res;
            const auto & m = (*this);
            res.x = m[0]*point.x + m[1]*point.y + m[2];
            res.y = m[3]*point.x + m[4]*point.y + m[5];
            return res;
        }

        void fill_diagonal(const_type_ref value) {
            index next = 0;
            for (index row = 0; row < this->rows(); ++row, next+=3)
                this->_data[next++] = value;
        }

        matrix_ref identity_fill() {
            this->fill(0);
            number fill_one{1};
            fill_diagonal(fill_one);
            return *this;
        }

        bool isIdentity() const {
            number zero=number{0}, one{1};
            return (
                this->_data[0]==one  && this->_data[1]==zero && this->_data[2]==zero &&
                this->_data[3]==zero && this->_data[4]==one  && this->_data[5]==zero &&
                this->_data[6]==zero && this->_data[7]==zero && this->_data[8]==one);
        }

    };

}
