#ifndef RAYTRACING_MATRIX_H
#define RAYTRACING_MATRIX_H

#include "vector.h"

struct Matrix
{
    const static unsigned SIZE = 3;
    double m[SIZE][SIZE];

    Matrix() : Matrix(0.) {}
    Matrix(const double diagonal);
};

Vector operator*(const Vector& lhs, const Matrix& rhs);
inline void operator*=(Vector& lhs, const Matrix& rhs) { lhs = lhs*rhs; }

Matrix operator*(const Matrix& lhs, const Matrix& rhs);

Matrix Inverse(const Matrix& a);
double Determinant(const Matrix& a);

Matrix RotationAroundX(double angle);
Matrix RotationAroundY(double angle);
Matrix RotationAroundZ(double angle);

Matrix Transpose(const Matrix& a);

#endif //RAYTRACING_MATRIX_H
