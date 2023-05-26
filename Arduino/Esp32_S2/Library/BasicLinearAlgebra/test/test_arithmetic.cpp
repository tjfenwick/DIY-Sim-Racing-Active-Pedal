#include <gtest/gtest.h>

#include "../BasicLinearAlgebra.h"

using namespace BLA;

TEST(Arithmetic, BraceInitialisation)
{
    Matrix<2, 2> B{0.0, 45.34, 32.98, 1456.1222};

    EXPECT_FLOAT_EQ(B(0, 0), 0);
    EXPECT_FLOAT_EQ(B(0, 1), 45.34);
    EXPECT_FLOAT_EQ(B(1, 0), 32.98);
    EXPECT_FLOAT_EQ(B(1, 1), 1456.1222);

    Matrix<2, 2> C{1.54, 5.98};

    EXPECT_FLOAT_EQ(C(0, 0), 1.54);
    EXPECT_FLOAT_EQ(C(0, 1), 5.98);
    EXPECT_FLOAT_EQ(C(1, 0), 0.0);
    EXPECT_FLOAT_EQ(C(1, 1), 0.0);

    Matrix<3, 3> D{0};

    EXPECT_FLOAT_EQ(Norm(D), 0.0);
}

TEST(Arithmetic, Fill)
{
    Matrix<2, 2> A;
    A.Fill(0.0f);

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            EXPECT_FLOAT_EQ(A(i, j), 0);
        }
    }
}

TEST(Arithmetic, OnesTest)
{
    Matrix<2, 2> A = Zeros<2,2>();
    Matrix<2, 2> B = Ones<2,2>();

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            EXPECT_FLOAT_EQ(A(i, j), 0.0f);
            EXPECT_FLOAT_EQ(B(i, j), 1.0f);
        }
    }
}

TEST(Arithmetic, AdditionSubtraction)
{
    Matrix<3, 3> A = {3.25, 5.67, 8.67, 4.55, 7.23, 9.00, 2.35, 5.73, 10.56};

    Matrix<3, 3> B = {6.54, 3.66, 2.95, 3.22, 7.54, 5.12, 8.98, 9.99, 1.56};

    auto C = A + B;
    auto D = A - B;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            EXPECT_FLOAT_EQ(C(i, j), A(i, j) + B(i, j));
            EXPECT_FLOAT_EQ(D(i, j), A(i, j) - B(i, j));
        }
    }

    C -= B;
    D += B;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            EXPECT_FLOAT_EQ(C(i, j), D(i, j));
        }
    }
}

TEST(Arithmetic, ElementwiseOperations)
{
    Matrix<3, 3> A = {3.25, 5.67, 8.67, 4.55, 7.23, 9.00, 2.35, 5.73, 10.56};

    auto C = A + 2.5;
    auto D = A - 3.7;
    auto E = A * 1.2;
    auto F = A / 6.7;

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            EXPECT_FLOAT_EQ(C(i, j), A(i, j) + 2.5);
            EXPECT_FLOAT_EQ(D(i, j), A(i, j) - 3.7);
            EXPECT_FLOAT_EQ(E(i, j), A(i, j) * 1.2);
            EXPECT_FLOAT_EQ(F(i, j), A(i, j) / 6.7);
        }
    }
}

TEST(Arithmetic, Multiplication)
{
    Matrix<3, 3> A = {3., 5., 8., 4., 7., 9., 2., 5.0, 10.};

    Matrix<3, 3> B = {6., 3., 2., 3., 7., 5., 8., 9., 1.};

    auto C = A * B;

    EXPECT_FLOAT_EQ(C(0, 0), 97.);
    EXPECT_FLOAT_EQ(C(0, 1), 116.);
    EXPECT_FLOAT_EQ(C(0, 2), 39.);
    EXPECT_FLOAT_EQ(C(1, 0), 117.);
    EXPECT_FLOAT_EQ(C(1, 1), 142.);
    EXPECT_FLOAT_EQ(C(1, 2), 52.);
    EXPECT_FLOAT_EQ(C(2, 0), 107);
    EXPECT_FLOAT_EQ(C(2, 1), 131.);
    EXPECT_FLOAT_EQ(C(2, 2), 39.);

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            EXPECT_FLOAT_EQ(C(i, j), (A.Row(i) * B.Column(j))(0, 0));
        }
    }

    A *= B;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            EXPECT_FLOAT_EQ(A(i, j), C(i, j));
        }
    }
}

TEST(Arithmetic, Concatenation)
{
    Matrix<3, 3> A = {3.25, 5.67, 8.67, 4.55, 7.23, 9.00, 2.35, 5.73, 10.56};

    Matrix<3, 3> B = {6.54, 3.66, 2.95, 3.22, 7.54, 5.12, 8.98, 9.99, 1.56};

    Matrix<3, 6> AleftOfB = A || B;
    Matrix<6, 3> AonTopOfB = A && B;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            EXPECT_FLOAT_EQ(AleftOfB(i, j), A(i, j));
            EXPECT_FLOAT_EQ(AleftOfB(i, j + 3), B(i, j));
            EXPECT_FLOAT_EQ(AonTopOfB(i, j), A(i, j));
            EXPECT_FLOAT_EQ(AonTopOfB(i + 3, j), B(i, j));
        }
    }
}

TEST(Arithmetic, OuterProduct)
{
    Matrix<3> v = {1.0, 2.0, 3.0};

    Matrix<3, 3> A = v * ~v;

    for (int i = 0; i < A.Rows; ++i)
    {
        for (int j = 0; j < A.Cols; ++j)
        {
            EXPECT_FLOAT_EQ(A(i, j), v(i) * v(j));
        }
    }
}

TEST(Arithmetic, Reference)
{
    const Matrix<3, 3> A = {3.25, 5.67, 8.67, 4.55, 7.23, 9.00, 2.35, 5.73, 10.56};
    Matrix<3, 3> B = {2.25, 6.77, 9.67, 14.55, 0.23, 3.21, 5.67, 6.75, 11.56};

    const auto A_ref = A.Submatrix<2, 2>(0, 1);
    auto B_ref = B.Submatrix<2, 2>(0, 1);

    B_ref(0, 0) = A(0, 0) * A_ref(0, 0);

    EXPECT_FLOAT_EQ(B(0, 1), A(0, 0) * A(0, 1));

    B.Submatrix<3, 3>(0, 0) = A;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            EXPECT_FLOAT_EQ(B(i, j), A(i, j));
        }
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
