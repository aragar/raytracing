#ifndef RAYTRACING_MATRIX_H
#define RAYTRACING_MATRIX_H

#include "vector.h"

struct Matrix
{
    const static unsigned SIZE = 3;
    float m[SIZE][SIZE];

    Matrix() :Matrix(0.f) {}
    Matrix(const float diagonal);
};

Vector operator*(const Vector& lhs, const Matrix& rhs);
void operator*=(Vector& lhs, const Matrix& rhs) { lhs = lhs*rhs; }

Matrix operator*(const Matrix& lhs, const Matrix& rhs);

Matrix Inverse(const Matrix& a);
float Determinant(const Matrix& a);

Matrix RotationAroundX(const float angle);
Matrix RotationAroundY(const float angle);
Matrix RotationAroundZ(const float angle);

#endif //RAYTRACING_MATRIX_H
