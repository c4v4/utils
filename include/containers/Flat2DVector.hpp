#ifndef _FLAT2DVECTOR_HPP
#define _FLAT2DVECTOR_HPP

#include "RandomIterator.hpp"
#include "VectorView.hpp"
#include "functors.hpp"
#include "noexception.hpp"

namespace cav {

    template <typename T>
    class Flat2DVector {

        ///////////// DATA STRUCTURE /////////////
    private:
        template <typename RowT>
        struct Rplus {
            RowT operator()(RowT vv, long int i) { return RowT(vv.begin() + vv.size() * i, vv.begin() + vv.size() * (i + 1)); }
        };

        template <typename RowT>
        struct Rless {
            bool operator()(RowT v1, RowT v2) { return v1.begin() < v2.begin(); }
        };

        template <typename RowT>
        struct Rminus {
            long int operator()(RowT v1, RowT v2) { return v1.begin() - v2.begin(); }
        };

    public:
        using Row = VectorView<T*>;
        using Const_Row = VectorView<const T*>;
        using iterator = RandomIterator<Row, Rplus, Rless, Rminus, identity_ftor>;
        using const_iterator = RandomIterator<Const_Row, Rplus, Rless, Rminus, identity_ftor>;


        ///////////// METHODS /////////////
    public:
        Flat2DVector() : data(nullptr), rows(0), cols(0) { }

        Flat2DVector(size_t rows_, size_t cols_) : data(new T[rows_ * cols_]), rows(rows_), cols(cols_) {
            if (data == nullptr) { _throw(std::runtime_error("Error: new returned nullptr inside Flat2DVector::Flat2DVector.")); }
        }

        Flat2DVector(size_t rows_, size_t cols_, T val = T()) : data(new T[rows_ * cols_]), rows(rows_), cols(cols_) {
            if (data == nullptr) { _throw(std::runtime_error("Error: new returned nullptr inside Flat2DVector::Flat2DVector.")); }
            std::fill(data, data + rows * cols, val);
        }

        ~Flat2DVector() { delete[] data; }

        inline void reset(size_t rows_, size_t cols_) {
            size_t old_size = rows * cols;
            rows = rows_;
            cols = cols_;
            if (old_size > rows * cols) { return; }

            delete[] data;
            data = new T[rows * cols];
            if (data == nullptr) { _throw(std::runtime_error("Error: new returned nullptr inside Flat2DVector::resize.")); }
        }

        inline void reset(size_t rows_, size_t cols_, T val) {
            reset(rows_, cols_);
            std::fill(data, data + rows * cols, val);
        }

        inline Row operator[](size_t i) { return Row(data + i * cols, data + (i + 1) * cols); }

        inline const Row operator[](size_t i) const { return Row(data + i * cols, data + (i + 1) * cols); }

        inline T& at(size_t i, size_t j) {
            assert(i < rows && j < cols);
            return data[i * cols + j];
        }
        inline const T& at(size_t i, size_t j) const {
            assert(i < rows && j < cols);
            return data[i * cols + j];
        }

        iterator begin() { return iterator(Row(data, data + cols)); }
        iterator end() { return iterator(Row(data + rows * cols, data + rows * cols)); }

        const_iterator begin() const { return const_iterator(Const_Row(data, data + cols)); }
        const_iterator end() const { return const_iterator(Const_Row(data + rows * cols, data + rows * cols)); }


        ///////////// FIELDS /////////////
    private:
        T* data;
        size_t rows;
        size_t cols;
    };

}  // namespace cav

#endif