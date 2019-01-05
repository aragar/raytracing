#include "matrix.h"

Matrix::Matrix(const double diagonal)
{
    for ( unsigned i = 0; i < SIZE; ++i )
        for ( unsigned j = 0; j < SIZE; ++j )
            m[i][j] = (i == j ? diagonal : 0);
}

Vector operator*(const Vector& lhs, const Matrix& rhs)
{
    return Vector(
            lhs.x*rhs.m[0][0] + lhs.y*rhs.m[1][0] + lhs.z*rhs.m[2][0],
            lhs.x*rhs.m[0][1] + lhs.y*rhs.m[1][1] + lhs.z*rhs.m[2][1],
            lhs.x*rhs.m[0][2] + lhs.y*rhs.m[1][2] + lhs.z*rhs.m[2][2]
    );
}

Matrix operator*(const Matrix& lhs, const Matrix& rhs)
{
    Matrix result;
    for ( unsigned i = 0; i < Matrix::SIZE; ++i )
        for ( unsigned j = 0; j < Matrix::SIZE; ++j )
            for ( unsigned k = 0; k < Matrix::SIZE; ++k )
                result.m[i][j] += lhs.m[i][k]*rhs.m[k][j];

    return result;
}

double Determinant(const Matrix& a)
{
    return (
            a.m[0][0]*a.m[1][1]*a.m[2][2]
          - a.m[0][0]*a.m[1][2]*a.m[2][1]
          - a.m[0][1]*a.m[1][0]*a.m[2][2]
          + a.m[0][1]*a.m[1][2]*a.m[2][0]
          + a.m[0][2]*a.m[1][0]*a.m[2][1]
          - a.m[0][2]*a.m[1][1]*a.m[2][0]
    );
}

double cofactor(const Matrix& a, const unsigned ii, const unsigned jj)
{
    int rows[Matrix::SIZE - 1];
    int rc = 0;
    int cols[Matrix::SIZE - 1];
    int cc = 0;

    for ( unsigned i = 0; i < Matrix::SIZE; ++i )
        if ( i != ii )
            rows[rc++] = i;

    for ( unsigned j = 0; j < Matrix::SIZE; ++j )
        if ( j != jj )
            cols[cc++] = j;

    double t = (
            a.m[rows[0]][cols[0]]*a.m[rows[1]][cols[1]] -
            a.m[rows[1]][cols[0]]*a.m[rows[0]][cols[1]]
    );
    if ( (ii + jj) % 2 )
        t = -t;

    return t;
}

Matrix Inverse(const Matrix &a)
{
    const double d = Determinant(a);
    if ( fabs(d) < 1e-12 )
        return a;

    const double rD = 1. / d;

    Matrix result;
    for ( unsigned i = 0; i < Matrix::SIZE; ++i )
        for ( unsigned j = 0; j < Matrix::SIZE; ++j )
            result.m[i][j] = rD*cofactor(a, j, i);

    return result;
}

Matrix RotationAroundX(double angle)
{
    double S = sin(angle);
    double C = cos(angle);

    Matrix result(1.);
    result.m[1][1] = C;
    result.m[2][1] = S;
    result.m[1][2] = -S;
    result.m[2][2] = C;

    return result;
}

Matrix RotationAroundY(double angle)
{
    double S = sin(angle);
    double C = cos(angle);

    Matrix result(1.);
    result.m[0][0] = C;
    result.m[2][0] = -S;
    result.m[0][2] = S;
    result.m[2][2] = C;

    return result;
}

Matrix RotationAroundZ(double angle)
{
    double S = sin(angle);
    double C = cos(angle);

    Matrix result(1.);
    result.m[0][0] = C;
    result.m[1][0] = S;
    result.m[0][1] = -S;
    result.m[1][1] = C;

    return result;
}

Matrix Transpose(const Matrix& a)
{
    Matrix res;
    for (unsigned i = 0; i < Matrix::SIZE; i++)
        for (unsigned j = 0; j < Matrix::SIZE; j++)
            res.m[i][j] = a.m[j][i];

    return res;
}
