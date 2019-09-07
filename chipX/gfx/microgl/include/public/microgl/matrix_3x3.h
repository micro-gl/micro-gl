#pragma once

#include <microgl/matrix.h>

namespace microgl {

    template<typename T>
    class matrix_3x3 : public matrix<T, 3, 3> {
    private:
        using index = unsigned;
        using base__ =  matrix<T, 3, 3>;
        using type_ref = T &;
        using const_type_ref = const T &;
        using matrix_ref = matrix_3x3<T> &;
        using const_matrix_ref = const matrix_3x3<T> &;

        static const index SX = 0;
        static const index SY = 4;
        static const index TX = 2;
        static const index TY = 5;
        static const index SKEWX = 1;
        static const index SKEWY = 3;

    public:

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
            mat[SKEWX] = tan(angles);
            return mat;
        }

        static
        matrix_3x3 shear_y(const_type_ref angles) {
            matrix_3x3 mat{};
            mat[SKEWY] = tan(angles);
            return mat;
        }

        static
        matrix_3x3 rotation(const_type_ref angle) {
            matrix_3x3 mat{};

            const_type_ref cos_ = cos(angle);
            const_type_ref sin_ = sin(angle);

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

            const_type_ref cos_ = cos(angle);
            const_type_ref sin_ = sin(angle);

            mat[0] = cos_;
            mat[1] = -sin_;
            mat[2] = -cos_*px + sin_*py + px;
            mat[3] = sin_;
            mat[4] = cos_;
            mat[5] = -sin_*px - cos_*py + py;

            return mat;
        }

        explicit matrix_3x3() {
            identity_fill();
        };

        matrix_3x3(const std::initializer_list<T> &list) :
                            base__(list) {}

        matrix_3x3(const_type_ref fill_value) :
                base__(fill_value) {}

        matrix_3x3(const base__ & mat) :
                base__(mat) {}

        virtual ~matrix_3x3() = default;

        void fill_diagonal(const_type_ref value) {
            index next = 0;
            for (index row = 0; row < this->rows(); ++row, next+=3)
                this->_data[next++] = value;
        }

        matrix_ref identity_fill() {
            this->fill(0);
            fill_diagonal(1);
            return *this;
        }

    };

}
