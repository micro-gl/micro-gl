#pragma once

namespace microgl {

    template <typename T, unsigned M>
    class matrix {
        using index = unsigned;
        using matrix_ref = matrix<T, M> &;
        using const_matrix_ref = const matrix<T, M> &;
        using type_ref = T &;
        using const_type_ref = const T &;

    public:
        matrix(std::initializer_list<T> list) {
            index ix = 0;
            for(auto it = list.begin(); it!=list.end(); it++)
                _data[ix++] = it;
        }

        matrix(const_matrix_ref mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix++] = mat[ix];
        }

        ~matrix() {

        }

        static void multiply(const_matrix_ref m1,
                             const_matrix_ref m2,
                             const_matrix_ref result) {
            // I use iterative indices to avoid multiplications
            index row_start_index = 0;

            for(index row = 0; row < M; row++) {
                for(index col = 0; col < M; col++) {

                    T c = 0;
                    index ix_m = 0;
                    for(index ix = 0; ix < M; ix++) {
//                        const_type_ref a = _data[row*M + ix];
//                        const_type_ref b = mat[col + ix*M];

                        const_type_ref a = m1[row_start_index + ix];
                        const_type_ref b = m2[col + ix_m];
                        c += a*b;
                        ix_m += M;
                    }

                    result[row_start_index + col] = c;
                }

                row_start_index += M;
            }

        }

        matrix operator*(const_matrix_ref mat) {
            matrix result;
            multiply(*this, mat, result);
            return result;
        };

        matrix_ref operator*=(const_matrix_ref mat) {
            multiply(*this, mat, *this);
            return *this;
        };

        matrix_ref operator=(const_matrix_ref mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = mat[ix];

            return *this;
        };

        inline index to_index(index row, index col) const {
            return row*M + col;
        }

        type_ref operator()(index row, index col) {
            return _data[to_index(row, col)];
        };

        const_type_ref operator()(index row, index col) const {
            return _data[to_index(row, col)];
        };

        type_ref operator[](index ix) {
            return _data[ix];
        };

        const_type_ref operator[](index ix) const {
            return _data[ix];
        };

        bool operator==(const_matrix_ref mat) const {
            bool equals = true;
            for(index ix = 0; ix < _size; ix++) {
                if(_data[ix]!=mat[ix])
                    return false;
            }
            return true;
        };

        void diagonal(const_type_ref value) {
            for (index ix = 0; ix < M; ++ix) {
                (*this)(ix, ix) = value;
            }
        }

        void fill(const_type_ref value) {
            for (index ix = 0; ix < _size; ++ix)
                _data[ix] = 0;
        }

        index columns() const {
            return _cols;
        }

        index rows() const {
            return _rows;
        }

        index size() const {
            return _size;
        }

    private:
        T _data[M*M];
        index _cols = M;
        index _rows = M;
        index _size = M*M;
    };

}
