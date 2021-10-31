#pragma once

namespace microgl {
    // this is a column major matrix
    template <typename number, unsigned W, unsigned H>
    class matrix {
    protected:
        using index = unsigned;
        using matrix_ref = matrix<number, W, H> &;
        using const_matrix_ref = const matrix<number, W, H> &;
        using type_ref = number &;
        using const_type_ref = const number &;

        number _data[W * H];
        static constexpr index _cols = W;
        static constexpr index _rows = H;
        static constexpr index _size = W * H;

    public:
        explicit matrix() = default;

        template<class Iterable>
        matrix(const Iterable & list) {
            index ix = 0;
            for (const auto & item : list)
                _data[ix++] = number(item);
        }

        explicit
        matrix(const_type_ref fill_value) { fill(fill_value); }

        matrix(const_matrix_ref mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = mat[ix];
        }

        template <typename T2>
        matrix(const matrix<T2, W, H> mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = number(mat[ix]);
        }

        virtual ~matrix() = default;

        template<unsigned A, unsigned B, unsigned C>
        static
        matrix<number, C, B> multiply(
                        const matrix<number, A, B> & m1,
                        const matrix<number, C, A> & m2) {
            matrix<number, C, B> result;
            // I use iterative indices to avoid multiplications
            index m1_row_start_index = 0;
            index result_row_start_index = 0;
            for(index row = 0; row < B; row++) {
                for(index col = 0; col < C; col++) {
                    number c = 0;
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
        matrix<number, A, H> operator*(const matrix<number, A, W> & mat) const {
            auto result = multiply(*this, mat);
            return result;
        };

        template<unsigned A>
        matrix<number, A, H> operator*=(const matrix<number, A, W> & mat) {
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

        inline index to_index(index row, index col) const { return row * W + col; }
        type_ref operator()(index row, index col) { return _data[to_index(row, col)]; };
        const_type_ref operator()(index row, index col) const { return _data[to_index(row, col)]; };
        type_ref operator[](const index ix) { return _data[ix]; };
        const_type_ref operator[](const index ix) const { return _data[ix]; };

        bool operator==(const_matrix_ref mat) const {
            for(index ix = 0; ix < size(); ix++) {
                if(_data[ix]!=mat[ix])
                    return false;
            }
            return true;
        };

        void setColumn(const index column_index, const number (&val)[H]) {
            index ind = column_index;
            for (index ix = 0; ix < H; ++ix) {
                _data[ind] = val[ix];
                ind+=W;
            }
        }

        void setRow(const index row_index, const number (&val)[W]) {
            index ind = row_index*W;
            for (index ix = 0; ix < W; ++ix) {
                _data[ind+ix] = val[ix];
            }
        }

        matrix<number, H, W> transpose() {
            matrix<number, H, W> result{};
            for (index row = 0; row < H; ++row) {
                for (index col = 0; col < W; ++col) {
                    result(col, row) = (*this)(row, col);
                }
            }
            return result;
        }

        void fill(const_type_ref value) {
            for (index ix = 0; ix < size(); ++ix) _data[ix] = value;
        }

        inline index columns() const { return _cols; }
        inline index rows() const { return _rows; }
        inline index size() const { return _size; }
    };

    template <typename T, unsigned N>
    using column_vector = matrix<T, 1, N>;
    template <typename T, unsigned N>
    using row_vector = matrix<T, N, 1>;
}