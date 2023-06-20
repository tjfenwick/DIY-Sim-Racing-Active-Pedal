#pragma once

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "Arduino.h"

namespace BLA
{
template <int rows, int cols, class MemT>
Matrix<rows, cols, MemT>::Matrix(MemT &d) : storage(d)
{
}

template <int rows, int cols, class MemT>
template <typename... TAIL>
Matrix<rows, cols, MemT>::Matrix(typename MemT::elem_t head, TAIL... args)
{
    FillRowMajor(0, head, args...);
}

template <int rows, int cols, class MemT>
template <class opMemT>
Matrix<rows, cols, MemT>::Matrix(const Matrix<rows, cols, opMemT> &obj)
{
    *this = obj;
}

template <int rows, int cols, class MemT>
template <class opMemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator=(const Matrix<rows, cols, opMemT> &obj)
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) (*this)(i, j) = obj(i, j);

    return *this;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator=(typename MemT::elem_t arr[rows][cols])
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) (*this)(i, j) = arr[i][j];

    return *this;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::Fill(const typename MemT::elem_t &val)
{
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) (*this)(i, j) = val;

    return *this;
}

template <int rows, int cols, class MemT>
template <typename... TAIL>
void Matrix<rows, cols, MemT>::FillRowMajor(int start_idx, typename MemT::elem_t head, TAIL... tail)
{
    static_assert(rows * cols > sizeof...(TAIL), "Too many arguments passed to FillRowMajor");

    (*this)(start_idx / cols, start_idx % cols) = head;

    FillRowMajor(++start_idx, tail...);
}

template <int rows, int cols, class MemT>
void Matrix<rows, cols, MemT>::FillRowMajor(int start_idx)
{
    for (int i = start_idx; i < rows * cols; ++i)
    {
        (*this)(i / cols, i % cols) = 0.0;
    }
}

template <int rows, int cols, class MemT>
typename MemT::elem_t &Matrix<rows, cols, MemT>::operator()(int row, int col)
{
    return storage(row, col);
}

template <int rows, int cols, class MemT>
typename MemT::elem_t Matrix<rows, cols, MemT>::operator()(int row, int col) const
{
    return storage(row, col);
}

template <int rows, int cols, class MemT>
template <int subRows, int subCols>
Matrix<subRows, subCols, Reference<MemT>> Matrix<rows, cols, MemT>::Submatrix(int top, int left)
{
    Reference<MemT> ref(storage, top, left);
    return Matrix<subRows, subCols, Reference<MemT>>(ref);
}

template <int rows, int cols, class MemT>
template <int subRows, int subCols>
Matrix<subRows, subCols, ConstReference<MemT>> Matrix<rows, cols, MemT>::Submatrix(int top, int left) const
{
    ConstReference<MemT> ref(storage, top, left);
    return Matrix<subRows, subCols, ConstReference<MemT>>(ref);
}

template <int rows, int cols, class MemT>
Matrix<1, cols, Reference<MemT>> Matrix<rows, cols, MemT>::Row(int i)
{
    return Submatrix<1, cols>(i, 0);
}

template <int rows, int cols, class MemT>
Matrix<1, cols, ConstReference<MemT>> Matrix<rows, cols, MemT>::Row(int i) const
{
    return Submatrix<1, cols>(i, 0);
}

template <int rows, int cols, class MemT>
Matrix<rows, 1, Reference<MemT>> Matrix<rows, cols, MemT>::Column(int j)
{
    return Submatrix<rows, 1>(0, j);
}

template <int rows, int cols, class MemT>
Matrix<rows, 1, ConstReference<MemT>> Matrix<rows, cols, MemT>::Column(int j) const
{
    return Submatrix<rows, 1>(0, j);
}

template <int rows, int cols, class MemT>
template <int operandCols, class opMemT>
Matrix<rows, cols + operandCols, HorzCat<cols, MemT, opMemT>> Matrix<rows, cols, MemT>::operator||(
    const Matrix<rows, operandCols, opMemT> &obj) const
{
    HorzCat<cols, MemT, opMemT> ref(storage, obj.storage);
    return Matrix<rows, cols + operandCols, HorzCat<cols, MemT, opMemT>>(ref);
}

template <int rows, int cols, class MemT>
template <int operandRows, class opMemT>
Matrix<rows + operandRows, cols, VertCat<rows, MemT, opMemT>> Matrix<rows, cols, MemT>::operator&&(
    const Matrix<operandRows, cols, opMemT> &obj) const
{
    VertCat<rows, MemT, opMemT> ref(storage, obj.storage);
    return Matrix<rows + operandRows, cols, VertCat<rows, MemT, opMemT>>(ref);
}

template <int rows, int cols, class MemT>
template <class opMemT>
Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator+(
    const Matrix<rows, cols, opMemT> &obj) const
{
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> ret(*this);
    ret += obj;
    return ret;
}

template <int rows, int cols, class MemT>
template <class opMemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator+=(const Matrix<rows, cols, opMemT> &obj)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            storage(i, j) += obj.storage(i, j);
        }
    }

    return *this;
}

template <int rows, int cols, class MemT>
template <class opMemT>
Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator-(
    const Matrix<rows, cols, opMemT> &obj) const
{
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> ret(*this);
    ret -= obj;
    return ret;
}

template <int rows, int cols, class MemT>
template <class opMemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator-=(const Matrix<rows, cols, opMemT> &obj)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            storage(i, j) -= obj.storage(i, j);
        }
    }

    return *this;
}

template <int rows, int cols, class MemT>
template <int operandCols, class opMemT>
Matrix<rows, operandCols, Array<rows, operandCols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator*(
    const Matrix<cols, operandCols, opMemT> &operand) const
{
    Matrix<rows, operandCols, Array<rows, operandCols, typename MemT::elem_t>> ret;

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < operandCols; j++)
        {
            if (cols > 0) ret.storage(i, j) = storage(i, 0) * operand.storage(0, j);

            for (int k = 1; k < cols; k++) ret.storage(i, j) += storage(i, k) * operand.storage(k, j);
        }

    return ret;
}

template <int rows, int cols, class MemT>
template <class opMemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator*=(const Matrix<rows, cols, opMemT> &operand)
{
    Matrix<rows, cols, MemT> tmp(*this);
    *this = tmp * operand;
    return *this;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator-() const
{
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> ret;

    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) ret(i, j) = -(*this)(i, j);

    return ret;
}

template <int rows, int cols, class MemT>
Matrix<cols, rows, Trans<MemT>> Matrix<rows, cols, MemT>::operator~() const
{
    Trans<MemT> ref(storage);
    Matrix<cols, rows, Trans<MemT>> tmp(ref);

    return tmp;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator+=(const typename MemT::elem_t k)
{
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) storage(i, j) += k;

    return *this;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator-=(const typename MemT::elem_t k)
{
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) storage(i, j) -= k;

    return *this;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator*=(const typename MemT::elem_t k)
{
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) storage(i, j) *= k;

    return *this;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, MemT> &Matrix<rows, cols, MemT>::operator/=(const typename MemT::elem_t k)
{
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) storage(i, j) /= k;

    return *this;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator+(
    const typename MemT::elem_t k) const
{
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> ret(*this);
    ret += k;
    return ret;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator-(
    const typename MemT::elem_t k) const
{
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> ret(*this);
    ret -= k;
    return ret;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator*(
    const typename MemT::elem_t k) const
{
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> ret(*this);
    ret *= k;
    return ret;
}

template <int rows, int cols, class MemT>
Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> Matrix<rows, cols, MemT>::operator/(
    const typename MemT::elem_t k) const
{
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> ret(*this);
    ret /= k;
    return ret;
}

inline Print &operator<<(Print &strm, const int obj)
{
    strm.print(obj);
    return strm;
}

inline Print &operator<<(Print &strm, const float obj)
{
    strm.print(obj);
    return strm;
}

inline Print &operator<<(Print &strm, const char *obj)
{
    strm.print(obj);
    return strm;
}

inline Print &operator<<(Print &strm, const char obj)
{
    strm.print(obj);
    return strm;
}

// Stream inserter operator for printing to strings or the serial port
template <int rows, int cols, class MemT>
Print &operator<<(Print &strm, const Matrix<rows, cols, MemT> &obj)
{
    strm << '[';

    for (int i = 0; i < rows; i++)
    {
        strm << '[';

        for (int j = 0; j < cols; j++) strm << obj(i, j) << ((j == cols - 1) ? ']' : ',');

        strm << (i == rows - 1 ? ']' : ',');
    }
    return strm;
}

}  // namespace BLA
