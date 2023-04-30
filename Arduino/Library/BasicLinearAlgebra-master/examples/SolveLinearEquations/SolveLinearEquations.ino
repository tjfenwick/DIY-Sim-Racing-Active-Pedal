#include <BasicLinearAlgebra.h>

using namespace BLA;

void setup()
{
    Serial.begin(115200);

    // One common thing that you might like to do with matrices is to solve systems
    // of linear equations of the form: A * x = b.

    // Let's go ahead and declare a decent sized system of equations to work with:
    Matrix<6, 6> A = {16, 78, 50, 84, 70, 63, 2,  32, 33, 61, 40, 17, 96, 98, 50, 80, 78, 27,
                      86, 49, 57, 10, 42, 96, 44, 87, 60, 67, 16, 59, 53, 8,  64, 97, 41, 90};

    Matrix<6> b = {3, 99, 95, 72, 57, 43};

    // To solve for x you might be tempted to take the inverse of A then
    // calculate x like so: x = A^-1 * b

    // As it turns out though, actually taking the inverse of A to solve these kinds of equations is quite inefficient.
    // Textbooks often talk about x = A^-1 * b, but in practice we don't actually compute x this way.

    // Instead we use something called an LU decomposition (or if A has some special properties you can use some other
    // type of decomposition but we won't get into that here). LU decomposition factors an A matrix into a permutation
    // matrix, a lower and an upper triangular matrix:
    auto A_decomp = A;  // LUDecompose will destroy A here so we'll pass in a copy so we can refer back to A later
    auto decomp = LUDecompose(A_decomp);

    // You can take a look at these matrices if you like:
    decomp.P();  // P essentially rearranges the rows of the matrix that results when we multiply L by U
    decomp.L();  // Will have ones along its diagonal and zeros above it
    decomp.U();  // Will have zeros below its diagonal

    // And if we multiply them all together we'll recover the original A matrix:
    Serial << "reconstructed A: " << decomp.P() * decomp.L() * decomp.U() << "\n";

    // Once we've done the decomposition we can solve for x very efficiently:
    Matrix<6> x_lusolve = LUSolve(decomp, b);

    Serial << "x (via LU decomposition): " << x_lusolve << "\n";

    // We can also recompute x for a new b vector without having to repeat the decomposition:
    Matrix<6> another_b = {23, 19, 86, 3, 23, 90};
    x_lusolve = LUSolve(decomp, another_b);

    // If you really need the inverse of A you can still compute it and calculate x with it:
    auto A_inv = A;
    Invert(A_inv);
    Matrix<6> x_Ainvb = A_inv * b;

    Serial << "x (via inverse A): " << x_Ainvb;

    // Fun fact though, we actually calculate A_inv by running LUDecompose then calling LUSolve for each column in A.
    // This is actually no less efficient than other methods for calculating the inverse.
}

void loop() {}
