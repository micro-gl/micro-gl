#pragma once

#include <microgl/matrix.h>

namespace microgl {

    template<typename T, unsigned N>
    class quad_matrix : public matrix<T, N, N> {
    private:
        using index = unsigned;
        using base__ =  matrix<T, N, N>;
        using type_ref = T &;
        using const_type_ref = const T &;
        using matrix_ref = quad_matrix<T, N> &;
        using const_matrix_ref = const quad_matrix<T, N> &;

    public:
        explicit quad_matrix() = default;

        quad_matrix(const std::initializer_list<T> &list) :
                            base__(list) {
        }

        quad_matrix(const_type_ref fill_value) :
                            matrix<T, N, N>(fill_value) {
        }

        quad_matrix(const base__ & mat) :
                            matrix<T, N, N>(mat) {
        }

        virtual ~quad_matrix() = default;

        void fill_diagonal(const_type_ref value) {
            index next = 0;
            for (index row = 0; row < this->rows(); ++row, next+=N)
                this->_data[next++] = value;
        }

        matrix_ref identity() {
            this->fill(0);
            fill_diagonal(1);
            return *this;
        }

    };

}
