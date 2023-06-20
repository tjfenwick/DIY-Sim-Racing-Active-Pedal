#pragma once

namespace BLA
{
template <int rows, int cols = 1, class ElemT = float>
struct Array
{
    typedef ElemT elem_t;
    elem_t m[rows * cols];

    elem_t &operator()(int row, int col) { return m[row * cols + col]; }
    elem_t operator()(int row, int col) const { return m[row * cols + col]; }
};

template <class MemT>
struct Reference
{
    typedef typename MemT::elem_t elem_t;

    MemT &parent;
    int rowOffset, colOffset;

    Reference<MemT>(MemT &obj, int rowOff, int colOff) : parent(obj), rowOffset(rowOff), colOffset(colOff) {}

    elem_t &operator()(int row, int col) { return parent(row + rowOffset, col + colOffset); }
    elem_t operator()(int row, int col) const { return parent(row + rowOffset, col + colOffset); }
};

template <class MemT>
struct ConstReference
{
    typedef typename MemT::elem_t elem_t;

    const MemT &parent;
    int rowOffset, colOffset;

    ConstReference<MemT>(const MemT &obj, int rowOff, int colOff) : parent(obj), rowOffset(rowOff), colOffset(colOff) {}
    ConstReference<MemT>(const ConstReference<MemT> &obj)
        : parent(obj.parent), rowOffset(obj.rowOffset), colOffset(obj.colOffset)
    {
    }

    elem_t operator()(int row, int col) const { return parent(row + rowOffset, col + colOffset); }
};

template <class ElemT>
struct Eye
{
    typedef ElemT elem_t;

    elem_t operator()(int row, int col) const { return row == col; }
};

template <class ElemT>
struct Zero
{
    typedef ElemT elem_t;

    elem_t operator()(int row, int col) const { return 0; }
};

template <class ElemT>
struct One
{
    typedef ElemT elem_t;

    elem_t operator()(int row, int col) const { return 1; }
};

template <int cols, int tableSize, class ElemT>
struct Sparse
{
    typedef ElemT elem_t;
    const static int size = tableSize;
    elem_t end;

    struct Element
    {
        int row, col;
        ElemT val;

        Element() { row = col = -1; }

    } table[tableSize];

    elem_t &operator()(int row, int col)
    {
        int hash = (row * cols + col) % tableSize;

        for (int i = 0; i < tableSize; i++)
        {
            Element &item = table[(hash + i) % tableSize];

            if (item.row == -1 || item.val == 0)
            {
                item.row = row;
                item.col = col;
                item.val = 0;
            }

            if (item.row == row && item.col == col)
            {
                return item.val;
            }
        }

        return end;
    }

    elem_t operator()(int row, int col) const
    {
        int hash = (row * cols + col) % tableSize;

        for (int i = 0; i < tableSize; i++)
        {
            const Element &item = table[(hash + i) % tableSize];

            if (item.row == row && item.col == col)
            {
                return item.val;
            }
        }

        return 0;
    }
};

template <class MemT>
struct Trans
{
    typedef typename MemT::elem_t elem_t;
    const MemT &parent;

    Trans<MemT>(const MemT &obj) : parent(obj) {}
    Trans<MemT>(Trans<MemT> &obj) : parent(obj.parent) {}

    elem_t operator()(int row, int col) const { return parent(col, row); }
};

template <int leftCols, class LeftMemT, class RightMemT>
struct HorzCat
{
    typedef typename LeftMemT::elem_t elem_t;
    const LeftMemT &left;
    const RightMemT &right;

    HorzCat<leftCols, LeftMemT, RightMemT>(const LeftMemT &l, const RightMemT &r) : left(l), right(r) {}

    elem_t operator()(int row, int col) const { return col < leftCols ? left(row, col) : right(row, col - leftCols); }
};

template <int topRows, class TopMemT, class BottomMemT>
struct VertCat
{
    typedef typename TopMemT::elem_t elem_t;
    const TopMemT &top;
    const BottomMemT &bottom;

    VertCat<topRows, TopMemT, BottomMemT>(const TopMemT &t, const BottomMemT &b) : top(t), bottom(b) {}

    elem_t operator()(int row, int col) const { return row < topRows ? top(row, col) : bottom(row - topRows, col); }
};

template <int dim, class ElemT>
struct Permutation
{
    typedef ElemT elem_t;

    int idx[dim];

    elem_t operator()(int row, int col) const { return idx[col] == row; }
};

template <class MemT>
struct LowerTriangleOnesDiagonal
{
    typedef typename MemT::elem_t elem_t;

    const MemT &parent;

    LowerTriangleOnesDiagonal<MemT>(const MemT &obj) : parent(obj) {}

    elem_t operator()(int row, int col) const
    {
        if (row > col)
        {
            return parent(row, col);
        }
        else if (row == col)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
};

template <class MemT>
struct UpperTriangle
{
    typedef typename MemT::elem_t elem_t;

    const MemT &parent;

    UpperTriangle<MemT>(const MemT &obj) : parent(obj) {}

    elem_t operator()(int row, int col) const
    {
        if (row <= col)
        {
            return parent(row, col);
        }
        else
        {
            return 0;
        }
    }
};

}  // namespace BLA
