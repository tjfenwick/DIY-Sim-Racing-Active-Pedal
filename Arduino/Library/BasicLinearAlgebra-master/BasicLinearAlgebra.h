#pragma once

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ElementStorage.h"

namespace BLA
{
template <int rows, int cols = 1, class MemT = Array<rows, cols, float>>
class Matrix
{
   public:
    typedef MemT mem_t;
    const static int Rows = rows;
    const static int Cols = cols;

    MemT storage;

    // Constructors
    Matrix<rows, cols, MemT>() = default;

    Matrix<rows, cols, MemT>(MemT &d);

    template <class opMemT>
    Matrix<rows, cols, MemT>(const Matrix<rows, cols, opMemT> &obj);

    template <typename... TAIL>
    Matrix(typename MemT::elem_t head, TAIL... args);

    // Assignment
    template <class opMemT>
    Matrix<rows, cols, MemT> &operator=(const Matrix<rows, cols, opMemT> &obj);

    Matrix<rows, cols, MemT> &operator=(typename MemT::elem_t arr[rows][cols]);

    Matrix<rows, cols, MemT> &Fill(const typename MemT::elem_t &val);

    template <typename... TAIL>
    void FillRowMajor(int start_idx, typename MemT::elem_t head, TAIL... tail);
    void FillRowMajor(int start_idx);

    // Element Access
    typename MemT::elem_t &operator()(int row, int col = 0);

    typename MemT::elem_t operator()(int row, int col = 0) const;

    template <int subRows, int subCols>
    Matrix<subRows, subCols, Reference<MemT>> Submatrix(int top, int left);

    template <int subRows, int subCols>
    Matrix<subRows, subCols, ConstReference<MemT>> Submatrix(int top, int left) const;

    Matrix<1, cols, Reference<MemT>> Row(int i);
    Matrix<1, cols, ConstReference<MemT>> Row(int i) const;

    Matrix<rows, 1, Reference<MemT>> Column(int j);
    Matrix<rows, 1, ConstReference<MemT>> Column(int j) const;

    // Concatenation
    template <int operandCols, class opMemT>
    Matrix<rows, cols + operandCols, HorzCat<cols, MemT, opMemT>> operator||(
        const Matrix<rows, operandCols, opMemT> &obj) const;

    template <int operandRows, class opMemT>
    Matrix<rows + operandRows, cols, VertCat<rows, MemT, opMemT>> operator&&(
        const Matrix<operandRows, cols, opMemT> &obj) const;

    // Addition
    template <class opMemT>
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> operator+(const Matrix<rows, cols, opMemT> &obj) const;

    template <class opMemT>
    Matrix<rows, cols, MemT> &operator+=(const Matrix<rows, cols, opMemT> &obj);

    // Subtraction
    template <class opMemT>
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> operator-(const Matrix<rows, cols, opMemT> &obj) const;

    template <class opMemT>
    Matrix<rows, cols, MemT> &operator-=(const Matrix<rows, cols, opMemT> &obj);

    // Multiplication
    template <int operandCols, class opMemT>
    Matrix<rows, operandCols, Array<rows, operandCols, typename MemT::elem_t>> operator*(
        const Matrix<cols, operandCols, opMemT> &operand) const;

    template <class opMemT>
    Matrix<rows, cols, MemT> &operator*=(const Matrix<rows, cols, opMemT> &operand);

    // Negation
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> operator-() const;

    // Transposition
    Matrix<cols, rows, Trans<MemT>> operator~() const;

    // Elementwise Operations
    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> operator+(const typename MemT::elem_t k) const;

    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> operator-(const typename MemT::elem_t k) const;

    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> operator*(const typename MemT::elem_t k) const;

    Matrix<rows, cols, Array<rows, cols, typename MemT::elem_t>> operator/(const typename MemT::elem_t k) const;

    Matrix<rows, cols, MemT> &operator+=(const typename MemT::elem_t k);
    Matrix<rows, cols, MemT> &operator-=(const typename MemT::elem_t k);
    Matrix<rows, cols, MemT> &operator*=(const typename MemT::elem_t k);
    Matrix<rows, cols, MemT> &operator/=(const typename MemT::elem_t k);
};

template <int rows, int cols = 1, class ElemT = float>
using ArrayMatrix = Matrix<rows, cols, Array<rows, cols, ElemT>>;

template <int rows, int cols, class ElemT = float>
using ArrayRef = Reference<Array<rows, cols, ElemT>>;

template <int rows, int cols, class ParentMemT>
using RefMatrix = Matrix<rows, cols, Reference<ParentMemT>>;

template <int rows, int cols = rows, class ElemT = float>
using Identity = Matrix<rows, cols, Eye<ElemT>>;

template <int rows, int cols = 1, class ElemT = float>
using Zeros = Matrix<rows, cols, Zero<ElemT>>;

template <int rows, int cols = 1, class ElemT = float>
using Ones = Matrix<rows, cols, One<ElemT>>;

template <int rows, int cols, int tableSize, class ElemT = float>
using SparseMatrix = Matrix<rows, cols, Sparse<cols, tableSize, ElemT>>;

template <int dim, class ElemT = float>
using PermutationMatrix = Matrix<dim, dim, Permutation<dim, ElemT>>;

template <int rows, int cols, class MemT>
using LowerTriangularDiagonalOnesMatrix = Matrix<rows, cols, LowerTriangleOnesDiagonal<MemT>>;

template <int rows, int cols, class MemT>
using UpperTriangularMatrix = Matrix<rows, cols, UpperTriangle<MemT>>;

}  // namespace BLA

#include "impl/BasicLinearAlgebra.h"
#include "impl/NotSoBasicLinearAlgebra.h"
