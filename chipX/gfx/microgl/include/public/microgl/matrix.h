#pragma once

namespace microgl {

    template <typename T, unsigned W, unsigned H>
    class matrix {
    protected:
        using index = unsigned;
        using matrix_ref = matrix<T, W, H> &;
        using const_matrix_ref = const matrix<T, W, H> &;
        using type_ref = T &;
        using const_type_ref = const T &;

        T _data[W * H];
        index _cols = W;
        index _rows = H;
        index _size = W * H;

    public:
        explicit matrix() = default;

        matrix(const std::initializer_list<T> &list) {
            index ix = 0;
            for(auto it = list.begin(); it!=list.end() && ix < size(); it++)
                _data[ix++] = *it;
        }

        explicit
        matrix(const_type_ref fill_value) {
            fill(fill_value);
        }

        matrix(const_matrix_ref mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = mat[ix];
        }

        template <typename T2>
        matrix(const matrix<T2, W, H> mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = mat[ix];
        }

        virtual ~matrix() = default;

        template<unsigned A, unsigned B, unsigned C>
        static
        matrix<T, C, B> multiply(
                        const matrix<T, A, B> & m1,
                        const matrix<T, C, A> & m2) {

            matrix<T, C, B> result;
            // I use iterative indices to avoid multiplications
            index m1_row_start_index = 0;
            index result_row_start_index = 0;

            for(index row = 0; row < B; row++) {

                for(index col = 0; col < C; col++) {

                    T c = 0;
                    index ix_m = 0;

                    for(index ix = 0; ix < A; ix++) {
                        const_type_ref a = m1[m1_row_start_index + ix];
                        const_type_ref b = m2[col + ix_m];
                        c += a*b;
                        ix_m += C;
                    }

                    result[result_row_start_index + col] = c;
                }

                result_row_start_index += C;
                m1_row_start_index += A;
            }

            return result;
        }

        static
        void multiply(matrix_ref m1,
                      const_type_ref value) {
            for (int ix = 0; ix < m1.size(); ++ix)
                m1[ix] *= value;
        }

        template<unsigned A>
        matrix<T, A, H> operator*(const matrix<T, A, W> & mat) const {
            auto result = multiply(*this, mat);
            return result;
        };

        template<unsigned A>
        matrix<T, A, H> operator*=(const matrix<T, A, W> & mat) {
            (*this) = multiply(*this, mat);
            return *this;
        };

        matrix operator*(const_type_ref value) const {
            matrix result = *this;
            multiply(result, value);
            return result;
        };

        matrix operator*=(const_type_ref value) {
            multiply(*this, value);
            return *this;
        };

        matrix_ref operator=(const_matrix_ref mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = mat[ix];
            return *this;
        };

        inline index to_index(index row, index col) const {
            return row * W + col;
        }

        type_ref operator()(index row, index col) {
            return _data[to_index(row, col)];
        };

        const_type_ref operator()(index row, index col) const {
            return _data[to_index(row, col)];
        };

        type_ref operator[](const index ix) {
            return _data[ix];
        };

        const_type_ref operator[](const index ix) const {
            return _data[ix];
        };

        bool operator==(const_matrix_ref mat) const {
            bool equals = true;
            for(index ix = 0; ix < size(); ix++) {
                if(_data[ix]!=mat[ix])
                    return false;
            }
            return true;
        };

        matrix<T, H, W> transpose() {
            matrix<T, H, W> result{};

            for (index row = 0; row < H; ++row) {
                for (index col = 0; col < W; ++col) {
                    result(col, row) = (*this)(row, col);
                }

            }

            return result;
        }

        void fill(const_type_ref value) {
            for (index ix = 0; ix < size(); ++ix)
                _data[ix] = value;
        }

        inline index columns() const {
            return _cols;
        }

        inline index rows() const {
            return _rows;
        }

        inline index size() const {
            return _size;
        }

    };

    template <typename T, unsigned N>
    using vector = matrix<T, 1, N>;
}
