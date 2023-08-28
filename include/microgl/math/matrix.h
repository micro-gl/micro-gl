/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

namespace microgl {
    // this is a column major matrix
    /**
     *
     * @tparam number
     * @tparam W
     * @tparam H
     * @tparam column_major layout in memory [a1 a3]                    [a1 a2]
     *                                       [a2 a4] = column major vs  [a3 a4] = row major
     *                                       where data = [a1 a2 a3 a4]
     */
    template <typename number, unsigned H, unsigned W, bool column_major=true>
    class matrix {
    public:
        using index = unsigned int;
        using value_type = number;
        using matrix_ref = matrix<number, W, H> &;
        using const_matrix_ref = const matrix<number, W, H> &;
        using type_ref = number &;
        using const_type_ref = const number &;

    protected:
        number _data[W * H];
        static constexpr index _cols = W;
        static constexpr index _rows = H;
        static constexpr index _size = W * H;

    public:
        explicit matrix() = default;
        explicit matrix(const_type_ref fill_value) { fill(fill_value); }

        template<class Iterable>
        matrix(const Iterable & list) {
            index ix = 0;
            for (const auto & item : list)
                _data[ix++] = number(item);
        }

        matrix(const_matrix_ref mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = mat[ix];
        }

        template <typename T2>
        matrix(const matrix<T2, W, H, column_major> mat) {
            for(index ix = 0; ix < _size; ix++)
                _data[ix] = number(mat[ix]);
        }

        virtual ~matrix() = default;

        constexpr static bool isColumnMajor() { return column_major; };

        template<unsigned B, unsigned A, unsigned C,
                 bool column_major_left, bool column_major_right>
        static matrix<number, B, C, column_major_left> multiply(
                const matrix<number, B, A, column_major_left> & m1,
                const matrix<number, A, C, column_major_right> & m2) {
            matrix<number, B, C, column_major_left> m3;
            // I use iterative indices to avoid multiplications
            index m1_row_begin_off = 0; // begin of rows in left matrix
            index m2_col_begin_off = 0; // begin of columns in right matrix
            index m3_row_begin_off = 0;
            constexpr index stride_row_m1 = column_major_left ? 1 : B;
            constexpr index stride_col_m1 = column_major_left ? B : 1;
            constexpr index stride_col_m2 = column_major_right ? A : 1;
            constexpr index stride_row_m2 = column_major_right ? 1 : A;
            constexpr index stride_col_m3 = column_major_left ? B : 1;
            constexpr index stride_row_m3 = column_major_left ? 1 : B;
            for(index row = 0; row < B; ++row, m1_row_begin_off+=stride_row_m1, m2_col_begin_off=0, m3_row_begin_off+=stride_row_m3) {
                for(index col=0, m3_col_off=m3_row_begin_off; col < C; ++col, m3_col_off+=stride_col_m3, m2_col_begin_off+=stride_col_m2) {
                    number acc = 0;
                    // dot product
                    index m1_col_off=m1_row_begin_off, m2_row_off=m2_col_begin_off;
                    for(index ix=0; ix < A; ++ix, m1_col_off+=stride_col_m1, m2_row_off+=stride_row_m2) {
                        acc += m1[m1_col_off]*m2[m2_row_off];
                    }
                    m3[m3_col_off] = acc;
                }
            }
            return m3;
        }

        template<unsigned A, unsigned B, unsigned C>
        static matrix<number, C, B> multiply3232323(
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

        static void multiply(matrix_ref m1, const_type_ref value) {
            for (int ix = 0; ix < m1.size(); ++ix) m1[ix] *= value;
        }

        template<unsigned A>
        matrix<number, H, A, column_major> operator*(const matrix<number, W, A, column_major> & mat) const {
            auto result = multiply(*this, mat);
            return result;
        };

        template<unsigned A>
        matrix<number, H, A, column_major> operator*=(const matrix<number, W, A, column_major> & mat) {
            (*this) = multiply(*this, mat);
            return *this;
        };

        matrix operator*(const_type_ref value) const {
            matrix result = *this;
            multiply(result, value);
            return result;
        };

        matrix operator*=(const_type_ref value) {
            multiply(*this, value); return *this;
        };

        matrix_ref operator=(const_matrix_ref mat) {
            for(index ix = 0; ix < _size; ix++) _data[ix] = mat[ix];
            return *this;
        };

    private:
//        template<bool columnMajor> inline index _to_index(index row, index col) const {}
//        template<> inline index _to_index<true>(index row, index col) const { return row + col*H; }
//        template<> inline index _to_index<false>(index row, index col) const { return row*W + col; }

        inline index _to_index(index row, index col) const {
            if(column_major)
                return row + col*H;
            else
                return row*W + col;
        }
//        template<> inline index _to_index<true>(index row, index col) const { return row + col*H; }
//        template<> inline index _to_index<false>(index row, index col) const { return row*W + col; }

    public:
        inline index to_index(index row, index col) const { return _to_index(row, col);}
        type_ref operator()(index row, index col) { return _data[to_index(row, col)]; };
        const_type_ref operator()(index row, index col) const { return _data[to_index(row, col)]; };
        type_ref operator[](const index ix) { return _data[ix]; };
        const_type_ref operator[](const index ix) const { return _data[ix]; };

        bool operator==(const_matrix_ref mat) const {
            for(index ix = 0; ix < size(); ix++) {
                if(_data[ix]!=mat[ix]) return false;
            }
            return true;
        };

        void setColumn(const index column_index, const number (&val)[H]) {
            // a faster iterative algorithm
            index ind = column_major ? column_index*H : column_index;
            index stride = column_major ? 1 : H;
            for (index ix = 0; ix < H; ++ix, ind+=stride)
                _data[ind] = val[ix];
        }

        void setRow(const index row_index, const number (&val)[W]) {
            index ind = column_major ? row_index : row_index*W;
            index stride = column_major ? H : 1;
            for (index ix = 0; ix < W; ++ix, ind+=stride)
                _data[ind] = val[ix];
        }

        matrix<number, W, H> transpose() {
            matrix<number, W, H> result{};
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

        inline constexpr index columns() const { return _cols; }
        inline constexpr index rows() const { return _rows; }
        inline constexpr index size() const { return _size; }
        number * data() const { return _data; }
        number * data() { return _data; }
    };

    template <typename T, unsigned N> using column_vector = matrix<T, 1, N>;
    template <typename T, unsigned N> using row_vector = matrix<T, N, 1>;
}